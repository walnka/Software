#include "software/networking/proto_radio_sender.h"

ProtoRadioSender::ProtoRadioSender(uint8_t channel, int spi_speed) : radio(RF24(CE_PIN, CSN_PIN, spi_speed))
{
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

ProtoRadioSender::~ProtoRadioSender() {
}

void ProtoRadioSender::send(uint8_t address[RADIO_ADDR_LENGTH], std::string data)
{
    long unsigned int max_size = std::pow(2, sizeof(uint8_t)*8)*(1+RADIO_PACKET_PAYLOAD_SIZE);
    if (data.length() > max_size)
    {
        LOG(WARNING) << "[ProtoRadioSender] Sender is larger than the radio wrapper can support. Packet is dropped";
        return;
    }

    radio.openWritingPipe(address);
    uint8_t num_packets = static_cast<uint8_t>(data.length() / RADIO_PACKET_PAYLOAD_SIZE);
    uint8_t data_offset = static_cast<uint8_t>(static_cast<int>(data.length()) % RADIO_PACKET_PAYLOAD_SIZE);
    const char* data_ptr = data.data();
    for (uint8_t packet_index = 0; packet_index < num_packets; ++packet_index)
    {
        data_buffer[RADIO_PACKET_LENGTH_INDEX] = num_packets;
        data_buffer[RADIO_PACKET_OFFSET_INDEX] = static_cast<uint8_t>(data_offset);
        data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] = packet_index;

        memcpy(&data_buffer[RADIO_HEADER_SIZE], data_ptr, RADIO_PACKET_PAYLOAD_SIZE);
        data_ptr += RADIO_PACKET_PAYLOAD_SIZE;

        if (!radio.write(&data_buffer, RADIO_PACKET_SIZE))
        {
            LOG(WARNING) << "[ProtoRadioSender] Unable to send packet to address " << (unsigned long) address;
            return;
        }
    }
    data_buffer[RADIO_PACKET_LENGTH_INDEX] = num_packets;
    data_buffer[RADIO_PACKET_OFFSET_INDEX] = data_offset;
    data_buffer[RADIO_PACKET_SEQUENCE_NUM_INDEX] = num_packets;
    memcpy(&data_buffer[RADIO_HEADER_SIZE], data_ptr, data_offset);

    if (!radio.write(&data_buffer, RADIO_PACKET_SIZE))
    {
        LOG(WARNING) << "[ProtoRadioSender] Unable to send packet to address " << (unsigned long)  address;
    }
}
