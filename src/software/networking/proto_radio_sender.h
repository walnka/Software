#pragma once

#include <RF24/RF24.h>
#include <cstdint>
#include "software/logger/logger.h"
#include "software/constants.h"

class ProtoRadioSender
{
public:
    ProtoRadioSender(uint8_t channel, int spi_speed);
    virtual ~ProtoRadioSender();

    virtual void send(uint8_t address, std::string data);

private:
    RF24 radio;
    // Buffer to hold serialized protobuf data
    uint8_t data_buffer[RADIO_PACKET_SIZE];

    static const uint8_t CE_PIN = 1; // SPI Chip Enable pin
    static const uint8_t CSN_PIN = 0; // SPI Chip Select pin
};
