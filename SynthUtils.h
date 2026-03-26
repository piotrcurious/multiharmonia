#ifndef SYNTHUTILS_H
#define SYNTHUTILS_H

#include <Arduino.h>

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
 * @brief Calculate frequency based on cent offset from a base frequency.
 * Formula: f = f0 * 2^(cents / 1200)
 */
inline float freqFromCents(float base_freq, float cents_offset) {
  return base_freq * pow(2.0f, cents_offset / 1200.0f);
}

#endif // SYNTHUTILS_H
