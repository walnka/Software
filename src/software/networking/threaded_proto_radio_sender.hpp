#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <string>

#include "software/networking/proto_radio_sender.h"
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
    std::map<int, std::array<uint8_t, RADIO_ADDR_LENGTH>> write_index_to_address;
    std::map<int, std::string> write_index_to_data;
    bool data_available[RADIO_MAX_PROTO_TYPES];
    std::mutex data_mutex[RADIO_MAX_PROTO_TYPES];

    static const unsigned int POLL_INTERVAL_MS = 100;
};

template <class SendProtoT>
void ThreadedProtoRadioSender::sendProto(const SendProtoT &message)
{
    std::string proto_type = message.GetDescriptor()->full_name();
    if (protobuf_to_write_index.find(proto_type) == protobuf_to_write_index.end())
    {
        LOG(WARNING) << proto_type << " is not registered with ThreadedProtoRadioSender";
        return;
    }

    int index = protobuf_to_write_index[proto_type];
    std::cout << "Index: " << (unsigned long) index << std::endl;
    std::mutex *m = &data_mutex[index];
    std::unique_lock data_lock(*m);
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
    std::cout << "Address from register sender: " << std::hex << (unsigned long) *address << std::endl;
    std::string proto_type = SendProtoT::descriptor()->full_name();


    if (num_open_writers >= RADIO_MAX_PROTO_TYPES)
    {
        LOG(WARNING) << "[ThreadedProtoRadioSender] Cannot send more than " << RADIO_MAX_PROTO_TYPES << " different proto types. Cannot register " << proto_type;
        return;
    }

    std::unique_lock radio_lock(radio_mutex);
    memcpy(&write_index_to_address[num_open_writers], address, RADIO_ADDR_LENGTH);

    //std::copy(std::begin(address), std::end(address), std::begin(write_index_to_address[proto_type]));
    //write_index_to_address[num_open_writers] = address;
    protobuf_to_write_index[proto_type] = num_open_writers;
    std::cout << "Index after writing: " << (unsigned long) *write_index_to_address[num_open_writers].data() << " Num open writers: " << (int) num_open_writers << std::endl;
    data_available[num_open_writers] = false;
    num_open_writers += 1;
    radio_lock.unlock();
}
