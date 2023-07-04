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
    radio.enableDynamicAck();
    
    radio.startListening();
};

ProtoRadioListener::~ProtoRadioListener() {
    radio.stopListening();
}

void ProtoRadioListener::receive() {
    uint8_t pipe;
    if (radio.available(&pipe)) {
        uint8_t bytes = radio.getDynamicPayloadSize();
        radio.read(&data_buffer, bytes);
        std::cout << "Recieved data on pipe: " << (int) pipe << "Data: " << data_buffer << std::endl;
        handleDataReception(pipe, bytes);
    }
    else {
      //std::cout << "No Radio Available" << std::endl;
    }
}

void ProtoRadioListener::registerListener(const uint8_t addr, std::function<void(std::string)> callback)
{
    auto pipe_it = std::find(std::begin(valid_pipes), std::end(valid_pipes), false);
    if (pipe_it == std::end(valid_pipes))
    {
        LOG(WARNING) << "A maximum of " << RADIO_MAX_PROTO_TYPES << " have already been registered";
    }

    std::size_t pipe_index = std::distance(std::begin(valid_pipes), pipe_it);
    valid_pipes[pipe_index] = true;
    pipe_to_callback[pipe_index] = callback;
    radio.openReadingPipe(static_cast<uint8_t>(pipe_index), addr);
    radio.printPrettyDetails();
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
        next_expected_packet_sequence_num = 0;
        expected_num_packets = data_buffer[RADIO_PACKET_LENGTH_INDEX];
        expected_offset = data_buffer[RADIO_PACKET_OFFSET_INDEX];
        assembled_data.clear();
    }

    assembled_data.write(&data_buffer[RADIO_HEADER_SIZE], buf_length-RADIO_HEADER_SIZE);

    if (data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] == expected_num_packets)
    {
        LOG(WARNING) << "Packet transfer succeeded!";
        pipe_to_callback[currently_reading_pipe](assembled_data.str());
    }

    next_expected_packet_sequence_num++;
}

bool ProtoRadioListener::isPacketInvalid(uint8_t rcvd_pipe, uint8_t buf_length) const
{
    LOG(INFO) << "is_currently_reading: " << (int) is_currently_reading;
    LOG(INFO) << "currently_reading_pipe: " << (int) currently_reading_pipe; 
    LOG(INFO) << "rcvd_pipe: " << (int) rcvd_pipe; 
    LOG(INFO) << "SEQUENCE_NUMBER on PACKET: " << (int) data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX];
    LOG(INFO) << "next expected sequence number: " << (int) next_expected_packet_sequence_num;
    LOG(INFO) << "PACKET_OFFSET_INDEX on packet: " << (int) data_buffer[RADIO_PACKET_OFFSET_INDEX];
    LOG(INFO) << "PACKETOFFSETLENGTH on packet: " << (int) data_buffer[RADIO_PACKET_LENGTH_INDEX];
    LOG(INFO) << "expected offset: " << (int) expected_offset;;
    LOG(INFO) << "expected num packets: " << (int) expected_num_packets;
    return (is_currently_reading && rcvd_pipe != currently_reading_pipe)
        || (!is_currently_reading && data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] != 0)
        || (is_currently_reading && data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] != next_expected_packet_sequence_num)
        || (is_currently_reading && data_buffer[RADIO_PACKET_OFFSET_INDEX] != expected_offset)
        || (is_currently_reading && data_buffer[RADIO_PACKET_LENGTH_INDEX] != expected_num_packets);
}
