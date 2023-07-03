#pragma once

#include "software/networking/proto_radio_listener.h"
#include "shared/constants.h"

#include <thread>
#include <unistd.h>

class ThreadedProtoRadioListener
{
public:
    ThreadedProtoRadioListener(uint8_t channel,
                               int spi_speed=1400000);

    ~ThreadedProtoRadioListener();

    template <class ReceiveProtoT>
    void registerListener(const uint8_t addr[RADIO_ADDR_LENGTH], std::function<void(ReceiveProtoT)> callback);
private:
    // The thread running the io_service in the background. This thread will run for the
    // entire lifetime of the class
    static const unsigned int POLL_INTERVAL_MS = 10;

    std::thread radio_listener_thread;
    ProtoRadioListener radio_listener;
};

template <class ReceiveProtoT>
void ThreadedProtoRadioListener::registerListener(const uint8_t addr[RADIO_ADDR_LENGTH], std::function<void(ReceiveProtoT)> callback)
{
    radio_listener.registerListener(addr, 
            [&](std::string raw_byte_data)
            {
                auto packet_data = ReceiveProtoT();
                if (!packet_data.ParseFromArray(raw_byte_data.data(), static_cast<int>(raw_byte_data.length())))
                {
                    LOG(WARNING) << "Packet of received type " << ReceiveProtoT::descriptor()->full_name() << " was corrupted";
                    return;
                }
                else
                {
                    LOG(INFO) << "ReceivedProtoT debug string: " << packet_data.DebugString();
                }
                
                callback(packet_data);
            });

}
