#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>

// USB HID
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};
Adafruit_USBD_HID usb_hid;

// Neopixel
Adafruit_NeoPixel pixels(1, 17, NEO_GRB + NEO_KHZ800);

// Array of keys to send (see https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h)
uint8_t hidcode[] = {HID_KEY_SCROLL_LOCK, HID_KEY_SCROLL_LOCK, HID_KEY_1, HID_KEY_ENTER};
uint32_t delays[] {0, 50, 100, 50, 100, 50, 100, 50};
uint8_t keycount = sizeof(hidcode) / sizeof(hidcode[0]);
uint8_t delaycount = sizeof(delays) / sizeof(delays[0]);

// Array of KVM inputs to cycle through
uint8_t input = 0;
uint8_t inputs[] = {HID_KEY_1, HID_KEY_2};
uint8_t inputcount = sizeof(inputs) / sizeof(inputs[0]);
bool stalled = false;

void hid_set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) report_id;
  (void) bufsize;
  if (report_type != HID_REPORT_TYPE_OUTPUT) return;
  // capslock = buffer[0] & KEYBOARD_LED_CAPSLOCK
}
uint16_t hid_get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  pixels.setPixelColor(0, pixels.Color(64, 64, 64));
  pixels.show();
  stalled = true;

  return 0;
}

void setup() {
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin();
  }

  pixels.begin();
  Serial1.begin(115200); // Setup the hardware serial port
  Serial.end(); // Make sure the USB HID device works with the KVM

  // Setup HID
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("ESPHome Ducky Keyboard");
  usb_hid.setReportCallback(hid_get_report_callback, hid_set_report_callback);
  usb_hid.begin();

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  // Setup input button
  pinMode(11, INPUT_PULLUP);
  
  // Setup neopixel
  pixels.clear();
  pixels.show();
}

void process() {
  static uint8_t keystate = 0;
  static uint32_t start = 0;

  if (keystate == 0) {
    pixels.setPixelColor(0, pixels.Color(128, 0, 0));
    pixels.show();
    if (false != digitalRead(11)) return;
    keystate = 1;
    Serial1.println("Started");

    input++;
    if (input >= inputcount) input = 0;
  } else if (keystate == ((keycount * 2) + 1)) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 128));
    pixels.show();
    if (false != digitalRead(11)) keystate = 0;
    return;
  }

  if (delaycount >= keystate) {
    if ((delays[keystate - 1] > 0) && ((millis() - start) <= delays[keystate - 1])) {
      if (!stalled) {
        pixels.setPixelColor(0, pixels.Color(0, 16, 0));
        pixels.show();
      }
      return;
    }
  }
  
  if (TinyUSBDevice.suspended()) {
    // Wake up the host if we're suspended
    TinyUSBDevice.remoteWakeup();
  }

  // Skip this operation if the USB HID isn't ready (it might be transfering a previous report)
  if (!usb_hid.ready()) {
      if (!stalled) {
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.show();
    }
    return;
  }

  if (!stalled) {
    pixels.setPixelColor(0, pixels.Color(0, 128, 0));
    pixels.show();
  }

  uint8_t const report_id = 0;
  if ((keystate & 0x1) == 1) {
    uint8_t const modifier = 0;
    uint8_t keycode[6] = {0};
    uint8_t hidindex = keystate >> 1;
    keycode[0] = hidindex == 2 ? inputs[input] : hidcode[hidindex];
    if (!usb_hid.keyboardReport(report_id, modifier, keycode)) {
      if (!stalled) {
        pixels.setPixelColor(0, pixels.Color(64, 64, 0));
        pixels.show();
        stalled = true;
      }
    }
  } else {
    if (!usb_hid.keyboardRelease(report_id)) {
      if (!stalled) {
        pixels.setPixelColor(0, pixels.Color(0, 64, 64));
        pixels.show();
        stalled = true;
      }
    }
  }
  start = millis();
  keystate++;
}

void loop() {
  TinyUSBDevice.task();
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  static uint32_t ms = 0;
  if (millis() - ms > 10) {
    ms = millis();
    process();
  }
}
