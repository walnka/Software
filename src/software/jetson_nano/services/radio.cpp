#include "software/jetson_nano/services/radio.h"

RadioService::RadioService()
    : 
{
    // TODO: construct listeners and packet trackers
}

void RadioService::primitiveSetCallback(TbotsProto::PrimitiveSet input)
{
    std::scoped_lock<std::mutex> lock(primitive_set_mutex);
    const uint64_t seq_num = input.sequence_number();

    primitive_set_tracker.send(seq_num);
    if (primitive_set_tracker.isLastValid())
    {
        primitive_set_msg = input;
    }

    float primitive_set_loss_rate = primitive_set_tracker.getLossRate();
    if (primitive_set_loss_rate > PROTO_LOSS_WARNING_THRESHOLD)
    {
        LOG(WARNING) << "Primitive set loss rate is " << primitive_set_loss_rate * 100
                     << "%";
    }
}

void RadioService::worldCallback(TbotsProto::World input)
{
    std::scoped_lock<std::mutex> lock(world_mutex);
    const uint64_t seq_num = input.sequence_number();

    world_tracker.send(seq_num);
    if (world_tracker.isLastValid())
    {
        world_msg = input;
    }

    float world_loss_rate = world_tracker.getLossRate();
    if (world_loss_rate > PROTO_LOSS_WARNING_THRESHOLD)
    {
        LOG(WARNING) << "World loss rate is " << world_loss_rate * 100 << "%";
    }
}
