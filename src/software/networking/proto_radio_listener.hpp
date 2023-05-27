#pragma once

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <cstdint>
#include <functional>

template <class ReceiveProtoT>
class ProtoRadioListener
{
public:
    ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
            uint8_t address, std::function<void(ReceiveProtoT)> receive_callback);
    virtual  ~ProtoRadioListener();
    void receive();
private:
    static const uint8_t ce_pin = 0; // SPI Chip Enable pin
    static const uint8_t csn_pin = 1; // SPI Chip Select pin
    RF24 radio;
    RF24Network network;

    // The function to call on every received packet of ReceiveProtoT data
    std::function<void(ReceiveProtoT&)> receive_callback;
};

template <class ReceiveProtoT>
ProtoRadioListener<ReceiveProtoT>::ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
                                                      uint8_t address, std::function<void(ReceiveProtoT)> receive_callback) :
        radio(RF24(ce_pin, csn_pin)), network(RF24Network(radio)), receive_callback(receive_callback){
    radio.setChannel(channel);
    radio.setAutoAck(false);
    network.begin(address);
    // Close unnecessary pipes
    // We only need the pipe opened for multicast listening
    for(uint8_t i = 0; i < 6; i++)
    {
        radio.closeReadingPipe(i);
    }

    network.multicastLevel(multicast_level);
};

template<class ReceiveProtoT>
ProtoRadioListener<ReceiveProtoT>::~ProtoRadioListener() {

}

template<class ReceiveProtoT>
void ProtoRadioListener<ReceiveProtoT>::receive() {
    network.update();
    while (network.available())
    {
        RF24NetworkHeader header;
        ReceiveProtoT payload;
        network.read(header, &payload, sizeof(payload));
        receive_callback(payload);
    }
}
