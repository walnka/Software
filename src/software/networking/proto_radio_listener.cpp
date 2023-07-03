#include "software/networking/proto_radio_listener.h"

ProtoRadioListener::ProtoRadioListener(uint8_t channel, int spi_speed)
    : radio(RF24(CE_PIN, CSN_PIN, spi_speed)),
    valid_pipes{}
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
    //radio.enableDynamicAck();
    
    radio.startListening();
    radio.printPrettyDetails();
};

ProtoRadioListener::~ProtoRadioListener() {
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

void ProtoRadioListener::registerListener(const uint8_t addr[RADIO_ADDR_LENGTH], std::function<void(std::string)> callback)
{
    auto pipe_it = std::find(std::begin(valid_pipes), std::end(valid_pipes), false);
    if (pipe_it == std::end(valid_pipes))
    {
        LOG(WARNING) << "A maximum of " << RADIO_MAX_PROTO_TYPES << " have already been registered";
    }

    std::size_t pipe_index = std::distance(std::begin(valid_pipes), pipe_it);
    valid_pipes[pipe_index] = true;
    pipe_to_callback[pipe_index] = callback;
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
        currently_reading_pipe = received_pipe;
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
        || (is_currently_reading && data_buffer[RADIO_PACKET_LENGTH_INDEX] != expected_num_packets);
}
