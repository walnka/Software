# Description:
#   RF24 is a OSI Layer 2 driver for nRF24L01
#   https://github.com/nRF24/RF24
#   https://nrf24.github.io/RF24/

# RF24 use the configure script to select a driver for the nRF24L01, so we need to run it before we can build
genrule(
    name = "RF24_configure",
    srcs = glob(["**/*"]),
    outs = [
        "utility/includes.h",
    ],
    cmd = "\n".join([
        "cd external/RF24",
        # Configure library to use SPIDEV as its physical driver
        "bash configure --driver=SPIDEV > /dev/null 2> /dev/null",
        # Need to be in base directory to use "make" variable
        "cd ../..",
        # Move the generated header to the location bazel expect it
        "mv external/RF24/utility/includes.h $@",
    ]),
)

cc_library(
    name = "RF24",
    hdrs = [
        "RF24.h",
        "RF24_config.h",
        "nRF24L01.h",
        # Explicitly require this header so that bazel knows to run the genrule to create it
        "utility/includes.h",
    ] + glob(
        ["utility/SPIDEV/*.h"],
        exclude = ["utility/SPIDEV/interrupt.h"],
    ),
    srcs = [
        "RF24.cpp",
    ] + glob(
        ["utility/SPIDEV/*.cpp"],
        exclude = ["utility/SPIDEV/interrupt.cpp"],
    ),
    includes = [
        "./utility",
    ],
    include_prefix = "RF24/",  # explicit so that RF24Network can find a header <RF24/RF24_config.h>
    defines = [
        "RF24_NO_INTERRUPT",  # disables RPi specific interrupts
    ],
    visibility = ["//visibility:public"],
)
