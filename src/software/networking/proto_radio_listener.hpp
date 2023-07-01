#pragma once

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <cstdint>
#include <functional>
#include "software/logger/logger.h"

class ProtoRadioListener
{
public:
    ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
            uint8_t address, std::function<void(ReceiveProtoT)> receive_callback);
    virtual  ~ProtoRadioListener();
    void receive();
private:
    static const uint8_t CE_PIN = 50;
    static const uint8_t CSN_PIN = 10;
    static const uint8_t SPI_SPEED = 1400000;
    RF24 radio;
//    RF24Network network;

    // The function to call on every received packet of ReceiveProtoT data
    bool valid_pipes[RADIO_MAX_PROTO_TYPES];
    std::function<void(std::string)> pipe_to_callback;
    uint8_t data_buffer[RADIO_PACKET_SIZE];
    std::stringstream assembled_data;

    bool is_currently_reading;
    uint8_t currently_reading_pipe;
    uint8_t next_expected_packet_sequence_num;
    uint8_t expected_num_packets;
    uint8_t expected_offset;
};

ProtoRadioListener<ReceiveProtoT>::ProtoRadioListener(uint8_t channel, uint8_t multicast_level,
                                                      uint8_t address, std::function<void(ReceiveProtoT)> receive_callback) :
        radio(RF24(CE_PIN, CSN_PIN, SPI_SPEED)),
        valid_pipes({ false })
{
    LOG(INFO) << "Initializing Radio Listener";
    try {
        if (!radio.begin()) {
            LOG(INFO) << "Radio hardware not responding!";
        }
    }
    catch (...)
    {
        LOG(INFO) << "proto_radio_listener.hpp: radio.begin() threw exception";
    }
    radio.setChannel(channel);
    radio.setPALevel(RF24_PA_MIN);
    radio.setAutoAck(false);
    radio.enableDynamicPayloads();

    radio.startListening();
    radio.printPrettyDetails();
};

ProtoRadioListener<ReceiveProtoT>::~ProtoRadioListener() {
    radio.stopListening();
}

void ProtoRadioListener::receive() {
    uint8_t pipe;
    if (radio.available(&pipe)) {
        uint8_t bytes = radio.getDynamicPayloadSize();
        radio.read(&data_buffer, bytes);
        handleDataReception(pipe, bytes);
    }
}

void ProtoRadioListener::handleDataReception(uint8_t received_pipe, uint8_t buf_length)
{
    if (!valid_pipes[received_pipe])
    {
        LOG(WARNING) << "Received data on pipe " << (int) received_pipe << " which was not registered";
        return;
    }

    if (isPacketInvalid(received_pipe, buf_length))
    {
        is_currently_reading = false;
        LOG(WARNING) << "Received an invalid packet on pipe " << (int) received_pipe;
        return;
    }

    if (!is_currently_reading)
    {
        is_currently_reading = true;
        currently_reading_pipe = pipe;
        expected_num_packets = data_buffer[RADIO_PACKET_LENGTH_INDEX];
        expected_offset = data_buffer[RADIO_PACKET_OFFSET_INDEX];
        assembled_data.clear();
    }

    assembled_data.write(&data_buffer[RADIO_HEADER_SIZE], buf_length-RADIO_HEADER_SIZE);

    if (data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] == expected_num_packets)
    {
        pipe_to_callback[currently_reading_pipe](assembled_data.str());
    }

    next_expected_packet_sequence_num++;
}

bool ProtoRadioListener::isPacketInvalid(uint8_t rcvd_pipe, uint8_t buf_length) const
{
    return (is_currently_reading && rcvd_pipe != currently_reading_pipe)
        || (!is_currently_reading && data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] != 0)
        || (is_currently_reading && data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] != next_expected_packet_sequence_num)
        || (is_currently_reading && data_buffer[RADIO_PACKET_OFFSET_INDEX] != expected_offset)
        || (is_currently_reading && data_buffer[RADIO_PACKET_LENGTH_INDEX] != expected_num_packets)
}
