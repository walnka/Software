#pragma once

#include "software/logger/logger.h"
#include "software/constants.h"

#include <RF24/RF24.h>
#include <cstdint>
#include <functional>

class ProtoRadioListener
{
public:
    ProtoRadioListener(uint8_t channel, int spi_speed);

    virtual ~ProtoRadioListener();
    void receive();
    
    void registerListener(const uint8_t addr[RADIO_ADDR_LENGTH], std::function<void(std::string)> callback);
private:

    bool isPacketInvalid(uint8_t rcvd_pipe, uint8_t buf_length) const;
    void handleDataReception(uint8_t received_pipe, uint8_t buf_length);

    static const uint8_t CE_PIN = 50;
    static const uint8_t CSN_PIN = 10;
    RF24 radio;

    // The function to call on every received packet of ReceiveProtoT data
    bool valid_pipes[RADIO_MAX_PROTO_TYPES];
    std::function<void(std::string)> pipe_to_callback[RADIO_MAX_PROTO_TYPES];
    char data_buffer[RADIO_PACKET_SIZE];
    std::stringstream assembled_data;

    bool is_currently_reading;
    uint8_t currently_reading_pipe;
    uint8_t next_expected_packet_sequence_num;
    uint8_t expected_num_packets;
    uint8_t expected_offset;
};
