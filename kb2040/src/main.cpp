#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>
#include "Ring.h"
#include "Ducky.h"

// Modes
bool mode_debug = false, mode_loopback = false;

// Buffers
Ring<char, 256> inputChars;
Ring<uint8_t, 256> inputNewlines;
Ring<DuckyKeyPress, 128> output;

// Ducky
Ducky ducky;

// USB HID
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};
Adafruit_USBD_HID usb_hid;

// Neopixel
Adafruit_NeoPixel pixels(1, 17, NEO_GRB + NEO_KHZ800);

void hid_set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) report_id;
  (void) bufsize;
}
uint16_t hid_get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;
  return 0;
}

void setup() {
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin();
  }

  Serial.end(); // Make sure the USB HID device works with devices that don't support serial over USB (e.g. KVMs)
  pixels.begin();
  Serial1.begin(115200); // Setup the hardware serial port
  if (mode_debug) Serial1.write("Hello from ESPHome Ducky Keyboard\n");

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

  // Setup neopixel
  pixels.clear();
  pixels.show();
}

void usb_loop() {
  if (TinyUSBDevice.suspended()) {
    // Wake up the host if we're suspended
    TinyUSBDevice.remoteWakeup();
  }
  if (!usb_hid.ready()) return;

  static uint8_t state = 0;
  static uint32_t start = 0;
  const uint8_t report_id = 0;
  switch (state) {
    case 0: {
      if (!output.is_empty()) {
        if (mode_debug) {
          pixels.setPixelColor(0, pixels.Color(0, 32, 0));
          pixels.show();
        }

        const DuckyKeyPress keyPress = output.peek();
        
        // Press the key with the modifier
        uint8_t const modifier = keyPress.modifier;
        uint8_t keycode[6] = {0};
        keycode[0] = keyPress.keycode;
        usb_hid.keyboardReport(report_id, modifier, keycode);
        if (mode_debug) Serial1.write("Pressed!\n");

        // Start the timer and move to state 1
        start = millis();
        state = 1;
      }
      break;
    }
    case 1: {
      const DuckyKeyPress keyPress = output.peek();
      if ((millis() - start) >= keyPress.pressTime) {
        if (mode_debug) {
          pixels.setPixelColor(0, pixels.Color(0, 0, 32));
          pixels.show();
        }

        // Release the key
        usb_hid.keyboardRelease(report_id);
        if (mode_debug) Serial1.write("Released!\n");
        // Restart the timer and move to state 2
        start = millis();
        state = 2;
      }
      break;
    }
    case 2: {
      const DuckyKeyPress keyPress = output.peek();
      if ((millis() - start) >= keyPress.afterTime) {
        // Read out the current key press, and recycle back to state 0
        output.read();

        if (mode_debug) {
          char buffer[128];
          snprintf(buffer, sizeof(buffer), "Resetting, %d keys remaining!", output.size());
          Serial1.println(buffer);
        }
        state = 0;
      }
      break;
    }
  }
}

void serial_loop() {
  static uint8_t nChars = 0;
  while ((Serial1.available() > 0) && !inputChars.is_full() && !inputNewlines.is_full()) {
    const char c = (char)Serial1.read();
    if (mode_loopback) Serial1.write(c);
    if (c == '\n') {
      try {
        inputNewlines.write(nChars);
      } catch (const invalid_state& e) {
        if (mode_debug || mode_loopback) {
          Serial1.write("Failed to record newline: ");
          Serial1.write(e.what());
          Serial1.write("\n");
        }
      }
      nChars = 0;
    } else nChars++;
    try {
      inputChars.write(c);
    } catch (const invalid_state& e) {
      if (mode_debug || mode_loopback) {
        Serial1.write("Failed to record character: ");
        Serial1.write(e.what());
        Serial1.write("\n");
      }
    }
  }
}

void ducky_loop() {
  while (!inputNewlines.is_empty()) {
    const uint8_t length = inputNewlines.peek();
    // Make sure we have enough space in the output buffer (assume a line results in fewer keypresses than keys, as is generally true)
    if (length <= output.space()) {
      // Read from the input buffer to a string, and discard the newline
      std::vector<char> v = inputChars.read(length);
      inputChars.read();
      inputNewlines.read();
      std::string string(v.begin(), v.end());

      if (string == "DEBUG") {
        mode_debug = !mode_debug;
        if (mode_debug) Serial1.println("Enabled debugging!");
        else {
          pixels.setPixelColor(0, pixels.Color(0, 0, 0));
          pixels.show();
        }
      } else if (string == "LOOPBACK") {
        mode_loopback = !mode_loopback;
        if (mode_loopback) Serial1.println("Enabled loopback!");
      } else {
        if (mode_debug) {
          char buffer[128];
          snprintf(buffer, sizeof(buffer), "Processing %d!", length);
          Serial1.println(buffer);
        }
        
        // Convert to key presses
        const std::vector<DuckyKeyPress> keyPresses = ducky.toKeys(string);
        output.write(keyPresses);

        if (mode_debug) {
          char buffer[128];
          snprintf(buffer, sizeof(buffer), "Created %d!", keyPresses.size());
          Serial1.println(buffer);
        }
      }
    } else break;
  }
}

void loop() {
  TinyUSBDevice.task();
  serial_loop();
  ducky_loop();
  if (TinyUSBDevice.mounted()) {
    static uint32_t usb_ms = 0;
    const uint32_t now = millis();
    if ((now - usb_ms) > 10) {
      usb_ms = now;
      usb_loop();
    }
  } else if (mode_debug) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
  }
}
