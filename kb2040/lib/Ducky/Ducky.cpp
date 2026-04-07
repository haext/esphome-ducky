#include "Ducky.h"

std::vector<DuckyKeyPress> Ducky::toKeys(std::string line) {
    std::vector<DuckyKeyPress> retVal;
    for (char c : line) {
        if (c >= 'A' && c <= 'Z') retVal.push_back(DuckyKeyPress{(uint8_t)((c - 'A') + HID_KEY_A), 50, 100});
        else if (c >= 'a' && c <= 'z') retVal.push_back(DuckyKeyPress{(uint8_t)((c - 'a') + HID_KEY_A), 50, 100});
        else if (c >= '1' && c <= '9') retVal.push_back(DuckyKeyPress{(uint8_t)((c - '1') + HID_KEY_1), 50, 100});
        else if (c == '0') retVal.push_back(DuckyKeyPress{HID_KEY_0, 50, 100});
    }
    return retVal;
}
