// Include the libraries for esp32 and ps2 keyboard
#include <Arduino.h>
#include <PS2Keyboard.h>
#include "SynthUtils.h"

// Define the pins for the analog knobs and the keyboard data and clock
#define KNOB1 34 // Base granularity of scale
#define KNOB2 35 // Base key of the scale quantified to selected granularity
#define KNOB3 36 // Intervals of the consonant key row
#define KNOB4 39 // Intervals of the dissonant key row
#define KNOB5 32 // Offset of the dissonant key row
#define KNOB6 33 // Offset of the additional consonant row
#define KNOB7 27 // Interval of additional consonant row
#define KNOB8 14 // Vector Row shift
#define DATA 16  // Keyboard data pin
#define CLOCK 17 // Keyboard clock pin

// Create an object for the keyboard
PS2Keyboard keyboard;

// Define the frequency range and the base frequency for the notes
#define MIN_FREQ 20   // Minimum frequency in Hz
#define MAX_FREQ 2000 // Maximum frequency in Hz
#define BASE_FREQ 440 // Base frequency in Hz

// Define the number of keys per row and the key codes for each row
#define KEYS_PER_ROW 10                  // Number of keys per row
const char CONSONANT_KEYS[KEYS_PER_ROW] = {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'}; // Consonant key row
const char DISSONANT_KEYS[KEYS_PER_ROW] = {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';'}; // Dissonant key row
const char ADDITIONAL_KEYS[KEYS_PER_ROW] = {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'}; // Additional consonant key row
const char PIVOTAL_KEYS[KEYS_PER_ROW] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};    // Pivotal key row

/*
The pivotal frequency row is an additional set of notes that are calculated from the geometric mean of the consonant and dissonant frequencies for each index. The geometric mean is a way of finding the average of two numbers that takes into account their relative proportions. For example, the geometric mean of 4 and 9 is 6, because 4 is to 6 as 6 is to 9. The geometric mean of two frequencies is also the frequency of the note that is halfway between them on a logarithmic scale. For example, the geometric mean of 440 Hz and 880 Hz is 623.5 Hz, which is the note E5, which is halfway between A4 and A5 on a piano keyboard.

The pivotal frequency row can be used to create interesting harmonies and melodies that blend the consonant and dissonant notes in a different way. The pivotal notes can also be seen as the points of balance or tension between the consonant and dissonant notes, depending on how they are played. You can experiment with different combinations of the knobs to create different scales and modes for the pivotal frequency row. 😊
*/

// Define a variable to store the current pressed key and its frequency
char current_key = '\0'; // Current pressed key
float current_freq = 0;  // Current frequency

// Define a function to calculate the frequency of a note based on its index and offset from the base frequency and the scale interval
float calculate_frequency(int index, float offset, float base_freq, float scale_interval, float vector_shift_cents) {
  float freq = base_freq * pow(scale_interval, index + offset);
  return freqFromCents(freq, vector_shift_cents);
}

// Define a function to calculate the pivotal frequency of a note based on its index and the consonant and dissonant frequencies
float calculate_pivotal_frequency(int index, float consonant_freq, float dissonant_freq) {
  return sqrt(consonant_freq * dissonant_freq); // Geometric mean of consonant and dissonant frequencies
}

// Define a function to play a note with a given frequency on the esp32 DAC output pin (GPIO25)
void play_tone(float freq) {
  ledcWriteTone(0, freq); // Write the frequency to channel 0
}

// Define a function to stop playing any note on the esp32 DAC output pin (GPIO25)
void stop_tone() {
  ledcWriteTone(0, 0); // Write zero frequency to channel 0
}

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  
  // Initialize the keyboard with the data and clock pins
  keyboard.begin(DATA, CLOCK);
  
  // Initialize the esp32 DAC output pin (GPIO25) with channel 0, resolution 8 bits and frequency 5000 Hz
  ledcSetup(0, 5000, 8);
  
  // Attach the esp32 DAC output pin (GPIO25) to channel 0
  ledcAttachPin(25, 0);
}

// Define static members of Oscillator
int16_t Oscillator::sineTable[SINE_TABLE_SIZE];
bool Oscillator::tableInitialized = false;

// Variables to store previous knob values for change detection
int prevKnob1 = -1, prevKnob2 = -1, prevKnob3 = -1, prevKnob4 = -1, prevKnob5 = -1, prevKnob6 = -1, prevKnob7 = -1, prevKnob8 = -1;
#define KNOB_THRESHOLD 50

// Monophonic voice management
MonoVoice voice;

// Knob values
float knob2_val, knob3_val, knob4_val, knob5_val, knob6_val, knob7_val, vector_shift_val;

float calc_current_freq(char key) {
  // Check if the key is one of the defined keys
  for (int i = 0; i < KEYS_PER_ROW; i++) {
    if (key == CONSONANT_KEYS[i]) {
      return calculate_frequency(i, 0, knob2_val, knob3_val, vector_shift_val);
    }
    else if (key == DISSONANT_KEYS[i]) {
      return calculate_frequency(i, knob5_val, knob2_val, knob4_val, vector_shift_val);
    }
    else if (key == ADDITIONAL_KEYS[i]) {
      return calculate_frequency(i, knob6_val, knob2_val, knob7_val, vector_shift_val);
    }
    else if (key == PIVOTAL_KEYS[i]) {
      float consonant_freq = calculate_frequency(i, 0, knob2_val, knob3_val, vector_shift_val);
      float dissonant_freq = calculate_frequency(i, knob5_val, knob2_val, knob4_val, vector_shift_val);
      return calculate_pivotal_frequency(i, consonant_freq, dissonant_freq);
    }
  }
  return 0;
}

void loop() {
  // Read raw knob values
  int k1 = analogRead(KNOB1);
  int k2 = analogRead(KNOB2);
  int k3 = analogRead(KNOB3);
  int k4 = analogRead(KNOB4);
  int k5 = analogRead(KNOB5);
  int k6 = analogRead(KNOB6);
  int k7 = analogRead(KNOB7);
  int k8 = analogRead(KNOB8);

  // Check if knobs moved significantly
  if (knobMoved(k1, prevKnob1, KNOB_THRESHOLD) ||
      knobMoved(k2, prevKnob2, KNOB_THRESHOLD) ||
      knobMoved(k3, prevKnob3, KNOB_THRESHOLD) ||
      knobMoved(k4, prevKnob4, KNOB_THRESHOLD) ||
      knobMoved(k5, prevKnob5, KNOB_THRESHOLD) ||
      knobMoved(k6, prevKnob6, KNOB_THRESHOLD) ||
      knobMoved(k7, prevKnob7, KNOB_THRESHOLD) ||
      knobMoved(k8, prevKnob8, KNOB_THRESHOLD)) {
    
    // Update mapped knob values
    float knob1 = mapf((float)k1, 0, 4095, MIN_FREQ, MAX_FREQ);
    knob2_val = quantize(mapf((float)k2, 0, 4095, MIN_FREQ, MAX_FREQ), knob1);
    knob3_val = mapf((float)k3, 0, 4095, 1.01, 2.00);
    knob4_val = mapf((float)k4, 0, 4095, 1.01, 2.00);
    knob5_val = mapf((float)k5, 0, 4095, -5, 5);
    knob6_val = mapf((float)k6, 0, 4095, -5, 5);
    knob7_val = mapf((float)k7, 0, 4095, 1.01, 2.00);
    vector_shift_val = mapf((float)k8, 0, 4095, -1200, 1200);

    // Print values
    Serial.print("Knob1: "); Serial.println(knob1);
    Serial.print("Knob2: "); Serial.println(knob2_val);
    Serial.print("Knob3: "); Serial.println(knob3_val);
    Serial.print("Knob4: "); Serial.println(knob4_val);
    Serial.print("Knob5: "); Serial.println(knob5_val);
    Serial.print("Knob6: "); Serial.println(knob6_val);
    Serial.print("Knob7: "); Serial.println(knob7_val);
    Serial.print("Vector: "); Serial.println(vector_shift_val);

    prevKnob1 = k1; prevKnob2 = k2; prevKnob3 = k3; prevKnob4 = k4;
    prevKnob5 = k5; prevKnob6 = k6; prevKnob7 = k7; prevKnob8 = k8;
  }

  // Update voice
  voice.update(keyboard, calc_current_freq);

  // Play tone if frequency changed
  if (voice.oscillator.frequency != current_freq) {
    current_freq = voice.oscillator.frequency;
    play_tone(current_freq);
    if (current_freq > 0) {
      Serial.print("Current key: "); Serial.println(voice.activeKey);
      Serial.print("Current freq: "); Serial.println(current_freq);
    }
  }

  delay(10);
}
