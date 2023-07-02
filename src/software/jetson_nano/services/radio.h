#pragma once

#include <mutex>
#include "proto/robot_status_msg.pb.h"
#include "software/networking/threaded_proto_radio_listener.hpp"
#include "software/networking/threaded_proto_radio_sender.hpp"
#include "proto/tbots_software_msgs.pb.h"
#include "proto/world.pb.h"
#include "software/jetson_nano/services/network/proto_tracker.h"
#include "software/logger/logger.h"

class RadioService
{
public:
    RadioService(uint8_t channel, uint8_t multicast_level, uint8_t address);
    std::tuple<TbotsProto::PrimitiveSet, TbotsProto::World> poll();
private:
    void worldCallback(TbotsProto::World input);
    void primitiveSetCallback(TbotsProto::PrimitiveSet input);

    // Variables
    TbotsProto::PrimitiveSet primitive_set_msg;
    TbotsProto::World world_msg;

    std::mutex primitive_set_mutex;
    std::mutex world_mutex;

    // std::unique_ptr<ProtoRadioSender<TbotsProto::RobotStatus>> sender;

    // radio receiver
    ThreadedProtoRadioListener threaded_proto_radio_listener;
    //std::unique_ptr<ThreadedProtoRadioListener<TbotsProto::World>> world_listener;
    //std::unique_ptr<ThreadedProtoRadioListener<TbotsProto::PrimitiveSet>> primitive_set_listener;

    // radio packet trackers
    ProtoTracker primitive_set_tracker;
    ProtoTracker world_tracker;

    static constexpr float PROTO_LOSS_WARNING_THRESHOLD = 0.1f;
};
