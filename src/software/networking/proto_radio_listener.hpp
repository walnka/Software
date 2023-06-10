#pragma once

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <cstdint>
#include <functional>
#include "software/logger/logger.h"

template <class ReceiveProtoT>
class ProtoRadioListener
{
public:
    ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
            uint8_t address, std::function<void(ReceiveProtoT)> receive_callback);
    virtual  ~ProtoRadioListener();
    void receive();
private:
//    static const uint8_t ce_pin = 0; // SPI Chip Enable pin
//    static const uint8_t csn_pin = 1; // SPI Chip Select pin
    static const uint8_t ce_pin = 50; // SPI Chip Enable pin
    static const uint8_t csn_pin = 10; // SPI Chip Select pin
    RF24 radio;
    RF24Network network;

    // The function to call on every received packet of ReceiveProtoT data
    std::function<void(ReceiveProtoT&)> receive_callback;
};

template <class ReceiveProtoT>
ProtoRadioListener<ReceiveProtoT>::ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
                                                      uint8_t address, std::function<void(ReceiveProtoT)> receive_callback) :
        radio(RF24(ce_pin, csn_pin)), network(RF24Network(radio)), receive_callback(receive_callback){
    LOG(INFO) << "Initializing Radio Listener";
    try {
        if (!radio.begin()) {
            LOG(INFO) << "Radio hardware not responding!";
        }
    }
    catch (...) {
        LOG(INFO) << "proto_radio_listener.hpp: radio.begin() threw exception";
    }
    radio.setChannel(channel);
    radio.setAutoAck(false);
    network.begin(address);
    // Close unnecessary pipes
    // We only need the pipe opened for multicast listening
//    for(uint8_t i = 0; i < 6; i++)
//    {
//        radio.closeReadingPipe(i);
//    }
    uint64_t addr = 0;
    radio.openReadingPipe(0, addr);
    radio.enableDynamicAck();
    radio.enableDynamicPayloads();

    network.multicastLevel(multicast_level);
    radio.startListening();
    LOG(INFO) << "Radio Listener Initialized";
    LOG(INFO) << "Network is valid" << network.is_valid_address(address);
};

template<class ReceiveProtoT>
ProtoRadioListener<ReceiveProtoT>::~ProtoRadioListener() {

}

template<class ReceiveProtoT>
void ProtoRadioListener<ReceiveProtoT>::receive() {
    if (radio.available()) {
        LOG(INFO) << "radio available";
    }
    network.update();
//    radio.printPrettyDetails();
    while (network.available())
    {
        LOG(INFO) << "inside loop";
        RF24NetworkHeader header;
        ReceiveProtoT payload;
        network.read(header, &payload, sizeof(payload));
        receive_callback(payload);
    }
}
