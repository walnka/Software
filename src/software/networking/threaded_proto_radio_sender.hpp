#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <string>

#include "software/networking/proto_radio_sender.hpp"
#include "shared/constants.h"

class ThreadedProtoRadioSender
{
public:
    /**
     *
     * @param channel
     * @param multicast_level
     * @param address
     */
    ThreadedProtoRadioSender(uint8_t channel, int spi_speed=1400000);

    ~ThreadedProtoRadioSender();


    template <class SendProtoT>
    void registerSender(const uint8_t address[RADIO_ADDR_LENGTH]);

    /**
     * Sends a protobuf message to the initialized ip address and port
     * This function returns after the message has been sent.
     *
     * @param message The protobuf message to send
     */
    template<class SendProtoT>
    void sendProto(const SendProtoT& message);

private:
    virtual void flushPipe();

    ProtoRadioSender radio_sender;

    // The thread running the io_service in the background. This thread will run for the
    // entire lifetime of the class
    std::thread radio_sender_thread;
    std::mutex radio_mutex;
    std::condition_variable radio_cv;

    int num_open_writers;
    int current_write_index;
    std::map<std::string, int> protobuf_to_write_index;
    std::map<int, uint8_t[5]> write_index_to_address;
    std::map<int, std::string> write_index_to_data;
    bool data_available[RADIO_MAX_PROTO_TYPES];
    std::mutex data_mutex[RADIO_MAX_PROTO_TYPES];

    static const unsigned int POLL_INTERVAL_MS = 100;
};

ThreadedProtoRadioSender::ThreadedProtoRadioSender(uint8_t channel, int spi_speed)
                                                             : radio_sender(channel, spi_speed),
                                                               num_open_writers(0),
                                                               current_write_index(0),
                                                               data_available{}
{
    radio_sender_thread = std::thread([this]() {
        for(;;) {
            std::unique_lock lock(radio_mutex);
            while(!data_available)
            {
                radio_cv.wait(lock);
            }
            for (int i = 0; i < num_open_writers; ++i)
            {
                flushPipe();
            }
            lock.unlock();
            usleep(POLL_INTERVAL_MS * MICROSECONDS_PER_MILLISECOND);
        }
    });
}

ThreadedProtoRadioSender::~ThreadedProtoRadioSender()
{
    radio_sender_thread.join();
}

void ThreadedProtoRadioSender::flushPipe()
{
    if (!data_available[current_write_index])
    {
        current_write_index = (current_write_index + 1) % num_open_writers;
        return;
    }

    radio_sender.send(write_index_to_address[current_write_index], write_index_to_data[current_write_index]);
    data_available[current_write_index] = false;

    current_write_index = (current_write_index + 1) % num_open_writers;
}

template <class SendProtoT>
void ThreadedProtoRadioSender::sendProto(const SendProtoT &message)
{
    std::string proto_type = message.GetDescriptor()->full_name();
    if (!protobuf_to_write_index.contains(proto_type))
    {
        LOG(WARNING) << proto_type << " is not registered with ThreadedProtoRadioSender";
        return;
    }

    int index = protobuf_to_write_index[proto_type];
    std::mutex m = data_mutex[index];
    std::unique_lock data_lock(m);
    message.SerializeToString(&write_index_to_data[index]);
    data_lock.unlock();

    std::unique_lock radio_lock(radio_mutex);
    data_available[index] = true;
    radio_lock.unlock();
    radio_cv.notify_one();
}

template<class SendProtoT>
void ThreadedProtoRadioSender::registerSender(const uint8_t address[RADIO_ADDR_LENGTH])
{
    std::string proto_type = SendProtoT::descriptor()->full_name();

    if (num_open_writers >= RADIO_MAX_PROTO_TYPES)
    {
        LOG(WARNING) << "[ThreadedProtoRadioSender] Cannot send more than " << RADIO_MAX_PROTO_TYPES << " different proto types. Cannot register " << proto_type;
    }

    std::unique_lock radio_lock(radio_mutex);
    protobuf_to_write_index[proto_type] = num_open_writers;
    write_index_to_address[num_open_writers] = address;
    data_available[num_open_writers] = false;
    num_open_writers += 1;
    radio_lock.unlock();
}
