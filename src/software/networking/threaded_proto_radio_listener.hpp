#pragma once

#include "software/networking/proto_radio_listener.hpp"
#include "shared/constants.h"

#include <thread>
#include <unistd.h>

template <class ReceiveProtoT>
class ThreadedProtoRadioListener
{
public:
    ThreadedProtoRadioListener(uint8_t channel, uint8_t multicast_level, uint8_t address,
                               std::function<void(ReceiveProtoT)> receive_callback);

    ~ThreadedProtoRadioListener();

private:
    // The thread running the io_service in the background. This thread will run for the
    // entire lifetime of the class
    static const unsigned int POLL_INTERVAL_MS = 10;
    ProtoRadioListener<ReceiveProtoT> radio_listener;
    std::thread radio_listener_thread;
};

template <class ReceiveProtoT>
ThreadedProtoRadioListener<ReceiveProtoT>::ThreadedProtoRadioListener(uint8_t channel, uint8_t multicast_level,
                                                                      uint8_t address, std::function<void(ReceiveProtoT)> receive_callback) :
    radio_listener(channel, multicast_level, address, receive_callback)
{
    // start the thread to run the io_service in the background
    std::cout << "starting radio listener thread" << std::endl;
    radio_listener_thread = std::thread([this]() {
        for (;;) {
            radio_listener.receive();
            usleep(POLL_INTERVAL_MS * MICROSECONDS_PER_MILLISECOND);
        }
    });
}

template <class ReceiveProtoT>
ThreadedProtoRadioListener<ReceiveProtoT>::~ThreadedProtoRadioListener()
{
    radio_listener_thread.join();
}

