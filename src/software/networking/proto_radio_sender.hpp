#pragma once

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <cstdint>

template <class SendProtoT>
class ProtoRadioSender
{
public:
    ProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                     uint8_t address);
    virtual  ~ProtoRadioSender();
    void sendProto(const SendProtoT& message);
private:
    static const uint8_t ce_pin = 0; // SPI Chip Enable pin
    static const uint8_t csn_pin = 1; // SPI Chip Select pin
    RF24 radio;
    RF24Network network;

    uint8_t multicast_level;

    // Buffer to hold serialized protobuf data
    std::string data_buffer;
};

template <class SendProtoT>
ProtoRadioSender<SendProtoT>::ProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                                              uint8_t address) : radio(RF24(ce_pin, csn_pin)), network(RF24Network(radio)), multicast_level(multicast_level)  {
    radio.setChannel(channel);
    network.begin(address);
    // Close unnecessary pipes
    // We only need the pipe opened for multicast listening
    for(int i = 0; i < 6; i++) {
        radio.closeReadingPipe(i);
    }

    network.multicastLevel(multicast_level);
};

template<class SendProtoT>
ProtoRadioSender<SendProtoT>::~ProtoRadioSender() {

}

template<class SendProtoT>
void ProtoRadioSender<SendProtoT>::sendProto(const SendProtoT& message) {
    // TODO: catch cant send spi message runtime error
    message.SerializeToString(&data_buffer);
    network.update();
    // multicast sets the receiving node to a multicast address for the header so we can just default construct it
    RF24NetworkHeader header;
    bool ok = network.multicast(header, data_buffer.data(), data_buffer.length(), multicast_level);
    // Potentially add delays
    if (!ok)
    {
        LOG(INFO) << "ProtoRadioSender sendProto failed";
    }
}
