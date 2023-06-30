#pragma once

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <cstdint>
#include "software/logger/logger.h"

class ProtoRadioSender
{
public:
    ProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                     uint8_t address);
    virtual ~ProtoRadioSender();

    virtual void send();

    virtual void transmit();

    template <class SendProtoT>
    void registerSender();

    template <class SendProtoT>
    void sendProto(const SendProtoT& message);
private:
    static const uint8_t CE_PIN = 2; // SPI Chip Enable pin
    static const uint8_t CSN_PIN = 0; // SPI Chip Select pin
    RF24 radio;

    uint8_t multicast_level;

    // Buffer to hold serialized protobuf data
    uint8_t data_buffer[RADIO_PACKET_SIZE];

    std::map<std::string, uint8_t[]> protobuf_name_to_address;
};

ProtoRadioSender::ProtoRadioSender(uint8_t channel, uint8_t multicast_level,
                                              uint8_t address) : radio(RF24(CE_PIN, CSN_PIN, 1400000)), multicast_level(multicast_level)  {
    LOG(INFO) << "Initializing Radio Sender";
    if (!radio.begin()) {
        LOG(INFO) << "Radio hardware not responding!";
    }

    radio.setChannel(channel);
    radio.setPALevel(RF24_PA_MIN);
    radio.setAutoAck(false);
    radio.enableDynamicPayloads();
    radio.stopListening();
    radio.printPrettyDetails();
};

ProtoRadioSender<SendProtoT>::~ProtoRadioSender() {
}

void ProtoRadioSender::send(uint8_t address[], std::string data)
{
    int max_size = std::pow(2, sizeof(uint8_t)*8)*(1+RADIO_PACKET_PAYLOAD_SIZE);
    if (data.length() > max_size)
    {
        LOG(WARNING) << "[ProtoRadioSender] Sender is larger than the radio wrapper can support. Packet is dropped";
        return
    }

    radio.openWritingPipe(address);
    int num_packets = data.length() / RADIO_PACKET_PAYLOAD_SIZE;
    int data_offset = data.length() % RADIO_PACKET_PAYLOAD_SIZE;
    const char* data_ptr = data.data();
    for (int packet_index = 0; packet_index < num_packets; ++packet_index)
    {
        data_buffer[RADIO_PACKET_LENGTH_INDEX] = num_packets;
        data_buffer[RADIO_PACKET_OFFSET_INDEX] = data_offset;
        data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] = packet_index;

        memcpy(&data_buffer[RADIO_HEADER_SIZE], data_ptr, RADIO_PACKET_PAYLOAD_SIZE);
        data_ptr += RADIO_PACKET_PAYLOAD_SIZE;

        if (!radio.write(&data_buffer, RADIO_PACKET_SIZE))
        {
            LOG(WARNING) << "[ProtoRadioSender] Unable to send packet of type " << proto_type;
            return;
        }

    }
    data_buffer[RADIO_PACKET_LENGTH_INDEX] = num_packets;
    data_buffer[RADIO_PACKET_OFFSET_INDEX] = data_offset;
    data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] = num_packets;
    memcpy(&data_buffer[RADIO_HEADER_SIZE], data_ptr, data_offset);
    if (!radio.write(&data_buffer, RADIO_PACKET_SIZE))
    {
        LOG(WARNING) << "[ProtoRadioSender] Unable to send packet of type " << proto_type;
        return;
    }
}
