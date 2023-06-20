#pragma once

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <cstdint>
#include "software/logger/logger.h"

template <class SendProtoT>
class ProtoRadioSender
{
public:
    ProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                     uint8_t address);
    virtual  ~ProtoRadioSender();
    void sendProto(const SendProtoT& message);
private:
//    static const uint8_t ce_pin = 0; // SPI Chip Enable pin
//    static const uint8_t csn_pin = 1; // SPI Chip Select pin
    static const uint8_t ce_pin = 2; // SPI Chip Enable pin
    static const uint8_t csn_pin = 0; // SPI Chip Select pin
    RF24 radio;
//    RF24Network network;

    uint8_t multicast_level;

    // Buffer to hold serialized protobuf data
    std::string data_buffer;
};

template <class SendProtoT>
ProtoRadioSender<SendProtoT>::ProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                                              uint8_t address) : radio(RF24(ce_pin, csn_pin)), multicast_level(multicast_level)  {
    LOG(INFO) << "Initializing Radio Sender";
    try {
        if (!radio.begin()) {
            LOG(INFO) << "Radio hardware not responding!";
        }
    }
    catch (...) {
        LOG(INFO) << "proto_radio_sender.hpp: radio.begin() threw exception";
    }
    radio.setChannel(channel);
    radio.setAutoAck(true);

//    // Close unnecessary pipes
//    // We only need the pipe opened for multicast listening
//    for(int i = 0; i < 6; i++) {
//        radio.closeReadingPipe(i);
//    }

    uint64_t addr = 1;
    uint64_t write_addr = 2;
    radio.openWritingPipe(write_addr);
    radio.openReadingPipe(1, addr);

    radio.enableDynamicAck();
    radio.enableDynamicPayloads();

    radio.stopListening();

//    network.begin(address);
//    network.multicastLevel(multicast_level);
    radio.printPrettyDetails();
};

template<class SendProtoT>
ProtoRadioSender<SendProtoT>::~ProtoRadioSender() {

}

template<class SendProtoT>
void ProtoRadioSender<SendProtoT>::sendProto(const SendProtoT& message) {
    // TODO: catch cant send spi message runtime error
//    message.SerializeToString(&data_buffer);
//    network.update();
    // multicast sets the receiving node to a multicast address for the header so we can just default construct it
//    RF24NetworkHeader header(1);
//    bool ok = network.multicast(header, data_buffer.data(), data_buffer.length(), multicast_level);
    // Potentially add delays
//    if (!ok)
//    {
//        LOG(INFO) << "ProtoRadioSender sendProto failed";
//        std::cout << "NOT OK" << std::endl;
//    } else {
//        std::cout << "SENT PROTO" << std::endl;
//    }
    uint64_t string_message = 0x12345;

    bool ok = radio.write(&string_message, sizeof(uint64_t));
    if (ok)
    {
        std::cout << "Message sent" << std::endl;
    } else {
        std::cout << "Message failed to send" << std::endl;
    }
}
