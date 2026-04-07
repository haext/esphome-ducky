#ifndef _DUCKY_H
#define _DUCKY_H

#include <stdint.h>
#include <string>
#include <vector>

#ifdef ARDUINO
    #include "Adafruit_TinyUSB.h"
#else
    // Copied from https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h with MIT license https://github.com/hathach/tinyusb/tree/master?tab=MIT-1-ov-file
    #define HID_KEY_A                           0x04
    #define HID_KEY_1                           0x1E
    #define HID_KEY_0                           0x27
#endif

class DuckyKeyPress {
    public:
        uint8_t hidcode;
        uint32_t pressTime;
        uint32_t afterTime;

        bool operator==(const DuckyKeyPress& other) const {
            return (hidcode == other.hidcode) && (pressTime == other.pressTime) && (afterTime == other.afterTime);
        }
};

class IDucky {
    public:
        virtual std::vector<DuckyKeyPress> toKeys(std::string line) = 0;
};

class Ducky : public IDucky {
    public:
        std::vector<DuckyKeyPress> toKeys(std::string line);
};


#endif