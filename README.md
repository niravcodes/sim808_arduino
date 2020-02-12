Modified from [Blemasle's library](https://github.com/blemasle/arduino-sim808) to add new functions like USSD codes and more precise GPS fields. I'll also try to find the time to write a nice documentation. I strongly recommend that you use the original library unless you know what you're doing.

# SIM808

This library allows to access some of the features of the [SIM808](https://simcom.ee/documents/?dir=SIM808) GPS & GPRS module. It requires only the `RESET` pin to work and a TTL Serial. `STATUS` pin can be wired to enhance the module power status detection, while wiring the `PWRKEY` adds the ability to turn the module on & off.

The library tries to reduces memory consumption as much as possible, but nonetheless use a 64 bytes buffer to communicate with the SIM808 module. When available, SIM808 responses are parsed to ensure that commands are correctly executed by the module. Commands timeouts are also set according to SIMCOM documentation.  

> No default instance is created when the library is included

[Arduino-Log](https://github.com/thijse/Arduino-Log) is used to output formatted commands in a `printf` style. This make implementation of new commands
really easy, and avoid successive prints or string concatenation on complex commands.

## Features
 * Fine control over the module power management
 * Sending SMS
 * Sending GET and POST [HTTP(s)](#a-note-about-https) requests
 * Acquiring GPS positions, with access to individual fields
 * Reading of the device states (battery, gps, network)

## Debugging
 If you need to debug the communication with the SIM808 module, you can either define `_DEBUG` to `1`, or directly change `_SIM808_DEBUG` to `1` in [SIMComAT.h](/src/SIMComAT.h).
 > Be aware that it will increase the final hex size as debug strings are stored in flash.

 ## Usage
 No default instance is created when the library is included. It's up to you to create one with the appropriate parameters.

 ```cpp
#include <SIM808.h>
#include <SoftwareSerial.h>

#define SIM_RST		5	///< SIM808 RESET
#define SIM_RX		6	///< SIM808 RXD
#define SIM_TX		7	///< SIM808 TXD
#define SIM_PWR		9	///< SIM808 PWRKEY
#define SIM_STATUS	8	///< SIM808 STATUS

#define SIM808_BAUDRATE 4800    ///< Control the baudrate use to communicate with the SIM808 module

SoftwareSerial simSerial = SoftwareSerial(SIM_TX, SIM_RX)
SIM808 sim808 = SIM808(SIM_RST, SIM_PWR, SIM_STATUS);
// SIM808 sim808 = SIM808(SIM_RST); // if you only have the RESET pin wired
// SIM808 sim808 = SIM808(SIM_RST, SIM_PWR); // if you only have the RESET and PWRKEY pins wired

void setup() {
    simSerial.begin(SIM808_BAUDRATE);
    sim808.begin(simSerial);

    sim808.powerOnOff(true);    //power on the SIM808. Unavailable without the PWRKEY pin wired
    sim808.init();
}

void loop() {
    // whatever you need to do
}
 ```
See examples for further usage.

## A note about HTTPS

While technically, SIM808 module support HTTPS requests through the HTTP service, it is particularly unreliable and sketchy. 7 times out of 10, the request won't succeed.  
In the future, I hope to find the time to make HTTPS work with the TCP service. In the meantime I strongly (and sadly) recommend to stick with HTTP requests if you need reliability.
