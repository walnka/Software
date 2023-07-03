#include "software/networking/threaded_proto_radio_listener.hpp"

ThreadedProtoRadioListener::ThreadedProtoRadioListener(uint8_t channel, int spi_speed)
    : radio_listener(channel, spi_speed)
{
    // start the thread to run the io_service in the background
    std::cout << "starting radio listener thread" << std::endl;
    radio_listener_thread = std::thread([this]() {
        for (;;) {
            radio_listener.receive();
            usleep(POLL_INTERVAL_MS * MICROSECONDS_PER_MILLISECOND);
        }
    });
}

ThreadedProtoRadioListener::~ThreadedProtoRadioListener()
{
    radio_listener_thread.join();
}
