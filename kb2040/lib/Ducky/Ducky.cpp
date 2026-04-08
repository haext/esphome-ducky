#include "Ducky.h"

uint8_t const ASCII2KEYCODE[128][2] = { HID_ASCII_TO_KEYCODE };

void ascii2Keys(std::vector<DuckyKeyPress> &retVal, const std::string line, const int start, const uint32_t pressTime, const uint32_t afterTime) {
    for (int i = start; i < line.size() ; i++) {
        const char c = line[i];
        if (c <= 0x7F) {
            const uint8_t keycode = ASCII2KEYCODE[c][1];
            const uint8_t modifier = ASCII2KEYCODE[c][0] ? KEYBOARD_MODIFIER_LEFTSHIFT : 0;
            retVal.push_back(DuckyKeyPress{keycode, modifier, pressTime, afterTime});
        }
    }
}

std::vector<DuckyKeyPress> Ducky::toKeys(std::string line) {
    std::vector<DuckyKeyPress> retVal;
    static const uint32_t pressTime = 50;
    static const uint32_t afterTime = 100;

    if (line.rfind("STRINGLN ", 0) == 0) {
        ascii2Keys(retVal, line, 9, pressTime, afterTime);
        retVal.push_back(DuckyKeyPress{HID_KEY_ENTER, 0, pressTime, afterTime});
    } else if (line.rfind("STRING ", 0) == 0) ascii2Keys(retVal, line, 7, pressTime, afterTime);
    else if (line == "CAPSLOCK") retVal.push_back(DuckyKeyPress{HID_KEY_CAPS_LOCK, 0, pressTime, afterTime});
    else if (line == "NUMLOCK") retVal.push_back(DuckyKeyPress{HID_KEY_NUM_LOCK, 0, pressTime, afterTime});
    else if (line == "SCROLLLOCK" || line == "SCROLLOCK") retVal.push_back(DuckyKeyPress{HID_KEY_SCROLL_LOCK, 0, pressTime, afterTime});
    return retVal;
}
