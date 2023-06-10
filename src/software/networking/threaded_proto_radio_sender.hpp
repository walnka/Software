#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <string>

#include "software/networking/proto_radio_sender.hpp"
#include "shared/constants.h"

template <class SendProto>
class ThreadedProtoRadioSender
{
public:
    /**
     *
     * @param channel
     * @param multicast_level
     * @param address
     */
    ThreadedProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                             uint8_t address);

    ~ThreadedProtoRadioSender();

    /**
     * Sends a protobuf message to the initialized ip address and port
     * This function returns after the message has been sent.
     *
     * @param message The protobuf message to send
     */
    void sendProto(const SendProto& message);

private:
    static const unsigned int POLL_INTERVAL_MS = 100;
    ProtoRadioSender<SendProto> radio_sender;
    SendProto radio_message;
    // The thread running the io_service in the background. This thread will run for the
    // entire lifetime of the class
    std::thread radio_sender_thread;
    std::mutex radio_mutex;
    std::condition_variable radio_cv;
};

template <class SendProtoT>
ThreadedProtoRadioSender<SendProtoT>::ThreadedProtoRadioSender(uint8_t channel,
                                                               uint8_t multicast_level,
                                                               uint8_t address) :
                                                               radio_sender(channel, multicast_level, address)
{
    radio_sender_thread = std::thread([this]() {
        for(;;) {
            std::unique_lock lock(radio_mutex);
            radio_cv.wait(lock);
            std::cout << radio_message.DebugString();
            radio_sender.sendProto(radio_message);
            lock.unlock();
            usleep(POLL_INTERVAL_MS * MICROSECONDS_PER_MILLISECOND);
        }
    });
}

template <class SendProtoT>
ThreadedProtoRadioSender<SendProtoT>::~ThreadedProtoRadioSender()
{
    radio_sender_thread.join();
}

template <class SendProtoT>
void ThreadedProtoRadioSender<SendProtoT>::sendProto(const SendProtoT &message)
{
    std::unique_lock lock(radio_mutex);
    radio_message = message;
    lock.unlock();
    radio_cv.notify_one();
}