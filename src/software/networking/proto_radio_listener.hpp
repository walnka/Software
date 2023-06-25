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
    uint64_t successCount = 0;
private:
//    static const uint8_t ce_pin = 0; // SPI Chip Enable pin
//    static const uint8_t csn_pin = 1; // SPI Chip Select pin
    static const uint8_t ce_pin = 77; // SPI Chip Enable pin
    static const uint8_t csn_pin = 11; // SPI Chip Select pin
    RF24 radio;
//    RF24Network network;

    // The function to call on every received packet of ReceiveProtoT data
    std::function<void(ReceiveProtoT&)> receive_callback;
};

template <class ReceiveProtoT>
ProtoRadioListener<ReceiveProtoT>::ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
                                                      uint8_t address, std::function<void(ReceiveProtoT)> receive_callback) :
        radio(RF24(ce_pin, csn_pin, 1400000)), receive_callback(receive_callback){
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
    radio.setPALevel(RF24_PA_MAX);
//    // Close unnecessary pipes
//    // We only need the pipe opened for multicast listening
//    for(uint8_t i = 0; i < 6; i++)
//    {
//        radio.closeReadingPipe(i);
//    }

    uint64_t addr = 2;
    // radio.openWritingPipe(1);
    radio.openReadingPipe(1, addr);
    // radio.openReadingPipe(0, addr);

    // radio.enableDynamicAck();
    radio.enableDynamicPayloads();

//    network.begin(address);
//    network.multicastLevel(multicast_level);
    radio.startListening();
    radio.printPrettyDetails();
};

template<class ReceiveProtoT>
ProtoRadioListener<ReceiveProtoT>::~ProtoRadioListener() {
    radio.stopListening();
}

template<class ReceiveProtoT>
void ProtoRadioListener<ReceiveProtoT>::receive() {
//    network.update();
//    while (network.available())
//    {
//        std::cout << "inside loop" << std::endl;
//        RF24NetworkHeader header;
//        ReceiveProtoT payload;
//        uint16_t payloadSize = network.peek(header);
//        network.read(header, &payload, payloadSize);
//        receive_callback(payload);
//    }

    uint8_t pipe;
    if (radio.available(&pipe)) {
        uint8_t bytes = radio.getDynamicPayloadSize();
        uint64_t payload;
        radio.read(&payload, bytes);
        std::cout << "Received " << (unsigned int) bytes;
        std::cout << " bytes on pipe" << (unsigned int) pipe;
        std::cout << ": " << payload << std::endl;
        successCount++;
        std::cout << "Successes " << (unsigned int) successCount << std::endl;
    } else {
        // std::cout << "RADIO UNAVAILABLE" << std::endl;
    }
}
