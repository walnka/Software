# Description:
#   RF24 is a OSI Layer 3 driver for nRF24L01
#   https://github.com/nRF24/RF24Network
#   https://nrf24.github.io/RF24Network/

cc_library(
    name = "RF24Network",
    hdrs = [
        "RF24Network.h",
        "RF24Network_config.h",
    ],
    srcs = [
        "RF24Network.cpp",
    ],
    deps = [
        "@RF24",
    ],
    include_prefix = "RF24Network/",  # Explicit because we have an include_prefix field for RF24
    visibility = ["//visibility:public"],
)
