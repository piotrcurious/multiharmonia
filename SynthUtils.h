#ifndef SYNTHUTILS_H
#define SYNTHUTILS_H

#include <Arduino.h>

// Special key codes for events
#define KEY_EVENT_PRESS   0x8000
#define KEY_EVENT_RELEASE 0x4000
#define KEY_CODE_MASK     0x0FFF

/**
 * @file SynthUtils.h
 * @brief Shared utilities for microtonal synthesizers on ESP32.
 */

/**
 * @brief Map a float value from one range to another.
 */
inline float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Map an integer value from one range to another.
 *
 * Standard Arduino map() implementation for integers.
 */
inline int mapValue(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
  if (fromHigh == fromLow) return toLow;
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

/**
 * @brief Quantize a frequency (or any float) to a given granularity.
 */
inline float quantize(float val, float granularity) {
  if (granularity <= 0) return val;
  return round(val / granularity) * granularity;
}

/**
 * @brief Read an analog pin and map it to a float range.
 */
inline float readKnobFloat(int pin, float min_val, float max_val) {
  int raw = analogRead(pin);
  return mapf((float)raw, 0, 4095, min_val, max_val);
}

/**
 * @brief Read an analog pin and map it to an integer range.
 */
inline int readKnobInt(int pin, int min_val, int max_val) {
  int raw = analogRead(pin);
  return mapValue(raw, 0, 4095, min_val, max_val);
}

/**
 * @brief Detect if a knob has moved beyond a certain threshold.
 */
inline bool knobMoved(int current, int previous, int threshold = 50) {
  return abs(current - previous) >= threshold;
}

/**
 * @brief A simple class to track keyboard state if the library doesn't.
 */
class NoteTracker {
public:
  bool states[256];
  NoteTracker() {
    for (int i = 0; i < 256; i++) states[i] = false;
  }

  void processEvent(uint16_t event) {
    uint8_t code = event & 0xFF;
    if (event & KEY_EVENT_PRESS) states[code] = true;
    else if (event & KEY_EVENT_RELEASE) states[code] = false;
  }

  bool isPressed(uint8_t code) {
    return states[code];
  }

  bool anyPressed() {
    for (int i = 0; i < 256; i++) if (states[i]) return true;
    return false;
  }
};

/**
 * @brief Calculate frequency based on cent offset from a base frequency.
 * Formula: f = f0 * 2^(cents / 1200)
 */
inline float freqFromCents(float base_freq, float cents_offset) {
  return base_freq * pow(2.0f, cents_offset / 1200.0f);
}

#endif // SYNTHUTILS_H
