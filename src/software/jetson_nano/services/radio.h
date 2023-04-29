#pragma once

class RadioService
{
public:
    RadioService();
    ~RadioService();
    std::tuple<TbotsProto::PrimitiveSet, TbotsProto::World> poll();
private:
    // radio receiver
};