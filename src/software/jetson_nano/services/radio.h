#pragma once

#include <mutex>
#include "software/networking/threaded_proto_radio_listener.hpp"
#include "proto/tbots_software_msgs.pb.h"
#include "proto/world.pb.h"
#include "software/jetson_nano/services/network/proto_tracker.h"
#include "software/logger/logger.h"

class RadioService
{
public:
    RadioService();
    ~RadioService();
    std::tuple<TbotsProto::PrimitiveSet, TbotsProto::World> poll();
private:
    void worldCallback(TbotsProto::World input);
    void primitiveSetCallback(TbotsProto::PrimitiveSet input);

    // radio receiver
    ThreadedProtoRadioListener<TbotsProto::World> world_listener;
    ThreadedProtoRadioListener<TbotsProto::PrimitiveSet> primitive_set_listener;

    // radio packet trackers
    ProtoTracker primitive_set_tracker;
    ProtoTracker world_tracker;

    static constexpr float PROTO_LOSS_WARNING_THRESHOLD = 0.1f;
};
