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

// Current active key in monophonic mode
char activeKey = '\0';

void loop() {
  
  // Read the values from the analog knobs and map them to their corresponding ranges
  
  // Base granularity of scale: from MIN_FREQ to MAX_FREQ in Hz
  float knob1 = readKnobFloat(KNOB1, MIN_FREQ, MAX_FREQ);
  
  // Base key of the scale quantified to selected granularity: from MIN_FREQ to MAX_FREQ in Hz, quantized by knob1 value
  float knob2 = quantize(readKnobFloat(KNOB2, MIN_FREQ, MAX_FREQ), knob1);
  
  // Intervals of the consonant key row: from 1.01 to 2.00 (multiplicative factor)
  float knob3 = readKnobFloat(KNOB3, 1.01, 2.00);
  
  // Intervals of the dissonant key row: from 1.01 to 2.00 (multiplicative factor)
  float knob4 = readKnobFloat(KNOB4, 1.01, 2.00);
  
  // Offset of the dissonant key row: from -5 to 5 (additive factor)
  float knob5 = readKnobFloat(KNOB5, -5, 5);
  
  // Offset of the additional consonant row: from -5 to 5 (additive factor)
  float knob6 = readKnobFloat(KNOB6, -5, 5);
  
  // Interval of additional consonant row: from 1.01 to 2.00 (multiplicative factor)
  float knob7 = readKnobFloat(KNOB7, 1.01, 2.00);
  
  // Print the values of the knobs to the serial monitor for debugging
  Serial.print("Knob1: "); Serial.println(knob1);
  Serial.print("Knob2: "); Serial.println(knob2);
  Serial.print("Knob3: "); Serial.println(knob3);
  Serial.print("Knob4: "); Serial.println(knob4);
  Serial.print("Knob5: "); Serial.println(knob5);
  Serial.print("Knob6: "); Serial.println(knob6);
  Serial.print("Knob7: "); Serial.println(knob7);
  
  // Check if a key is available from the keyboard
  if (keyboard.available()) {
    
    // Read the key from the keyboard
    char key = keyboard.read();
    
    // Check if the key is different from the current key
    if (key != activeKey) {
      
      // Update the current key
      activeKey = key;
      
      // Vector shift in cents
      float vector_shift = mapf(analogRead(KNOB8), 0, 4095, -1200, 1200);

      current_freq = 0;
      // Check if the key is one of the defined keys
      for (int i = 0; i < KEYS_PER_ROW; i++) {
        if (key == CONSONANT_KEYS[i]) {
          current_freq = calculate_frequency(i, 0, knob2, knob3, vector_shift);
          break;
        }
        else if (key == DISSONANT_KEYS[i]) {
          current_freq = calculate_frequency(i, knob5, knob2, knob4, vector_shift);
          break;
        }
        else if (key == ADDITIONAL_KEYS[i]) {
          current_freq = calculate_frequency(i, knob6, knob2, knob7, vector_shift);
          break;
        }
        else if (key == PIVOTAL_KEYS[i]) {
          float consonant_freq = calculate_frequency(i, 0, knob2, knob3, vector_shift);
          float dissonant_freq = calculate_frequency(i, knob5, knob2, knob4, vector_shift);
          current_freq = calculate_pivotal_frequency(i, consonant_freq, dissonant_freq);
          break;
        }
      }
      
      if (current_freq > 0) {
        play_tone(current_freq);
        Serial.print("Current key: "); Serial.println(activeKey);
        Serial.print("Current freq: "); Serial.println(current_freq);
      }
    }
    delay(10); // Add a small delay to avoid bouncing keys
  }

  // Check for release
  if (activeKey != '\0' && keyboard.readKeyState(activeKey) == 0) {
    stop_tone();
    activeKey = '\0';
  }
}
