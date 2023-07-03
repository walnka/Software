#include "software/networking/threaded_proto_radio_sender.hpp"

ThreadedProtoRadioSender::ThreadedProtoRadioSender(uint8_t channel, int spi_speed)
                                                             : radio_sender(channel, spi_speed),
                                                               num_open_writers(0),
                                                               current_write_index(0),
                                                               data_available{}
{
    radio_sender_thread = std::thread([this]() {
        for(;;) {
            std::unique_lock lock(radio_mutex);
            while(!data_available)
            {
                radio_cv.wait(lock);
            }
            for (int i = 0; i < num_open_writers; ++i)
            {
                flushPipe();
            }
            lock.unlock();
            usleep(POLL_INTERVAL_MS * MICROSECONDS_PER_MILLISECOND);
        }
    });
}

ThreadedProtoRadioSender::~ThreadedProtoRadioSender()
{
    radio_sender_thread.join();
}

void ThreadedProtoRadioSender::flushPipe()
{
    if (!data_available[current_write_index])
    {
        current_write_index = (current_write_index + 1) % num_open_writers;
        return;
    }

    uint8_t write_addr[RADIO_ADDR_LENGTH]; 
    std::cout << "Memcopy index to address: " << (unsigned long) *write_index_to_address[current_write_index].data() << std::endl;
    for (unsigned i = 0; i < RADIO_ADDR_LENGTH; ++i)
    {
      write_addr[i] = write_index_to_address[current_write_index][i];
    }
    std::cout << "Memcopy index to address: " << std::hex << (unsigned long) write_index_to_address[current_write_index].data()[0] << std::endl;
    std::cout << "Memcopy index to address: " << std::hex << (unsigned long) write_index_to_address[current_write_index].data()[1] << std::endl;
    std::cout << "Memcopy index to address: " << std::hex << (unsigned long) write_index_to_address[current_write_index].data()[2] << std::endl;
    std::cout << "Memcopy index to address: " << std::hex << (unsigned long) write_index_to_address[current_write_index].data()[3] << std::endl;
    std::cout << "Memcopy index to address: " << std::hex << (unsigned long) write_index_to_address[current_write_index].data()[4] << std::endl;
    std::cout << "Current write index: " << (unsigned long) current_write_index << std::endl;
    std::cout << "write_addr: " << std::hex << (unsigned long) *write_addr << std::endl;
    radio_sender.send(write_addr, write_index_to_data[current_write_index]);
    data_available[current_write_index] = false;

    current_write_index = (current_write_index + 1) % num_open_writers;
}

