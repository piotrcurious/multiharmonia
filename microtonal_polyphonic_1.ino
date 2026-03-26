// Include the libraries for esp32 and ps2 keyboard
#include <Arduino.h>
#include <PS2Keyboard.h>
#include "SynthUtils.h"

// Define the pins for the analog knobs and the keyboard data and clock
#define KNOB1 A0 // Base granularity of scale
#define KNOB2 A1 // Base key of the scale quantified to selected granularity
#define KNOB3 A2 // Intervals of the consonant key row
#define KNOB4 A3 // Intervals of the dissonant key row
#define KNOB5 A4 // Offset of the dissonant key row
#define KNOB6 A5 // Offset of the additional consonant row
#define KNOB7 A6 // Interval of additional consonant row
#define KNOB8 33 // Vector Row shift/transpose (cents)
#define DATA 16  // Keyboard data pin
#define CLOCK 17 // Keyboard clock pin

// Define the maximum number of notes that can be played simultaneously
#define MAX_NOTES 8

// Define the frequency range for the notes in Hz
#define MIN_FREQ 20
#define MAX_FREQ 20000

// Create an object for the ps2 keyboard
PS2Keyboard keyboard;

// Create an array to store the current notes being played
float notes[MAX_NOTES];

// Create an array to store the corresponding keyboard keys for each note
char keys[MAX_NOTES];

// Create an array to store whether a channel is active
bool active[MAX_NOTES];


// Helper to find key index in a row
int find_key_in_row(char key, const char* row) {
  const char* p = strchr(row, key);
  if (p) return p - row;
  return -1;
}

// Create a function to calculate the frequency of a note based on the knob values and the keyboard key
float calculate_frequency(char key) {
  // Read the knob values and constrain them to reasonable ranges
  float knob1 = constrain(readKnobFloat(KNOB1, 1, 100), 1.0f, 100.0f); // Base granularity of scale in cents (1/100 of a semitone)
  float knob2 = constrain(readKnobFloat(KNOB2, 0, knob1), 0.0f, knob1); // Base key of the scale quantified to selected granularity in cents
  float knob3 = constrain(readKnobFloat(KNOB3, knob1, knob1 * 12), knob1, knob1 * 12); // Intervals of the consonant key row in cents
  float knob4 = constrain(readKnobFloat(KNOB4, knob1, knob1 * 12), knob1, knob1 * 12); // Intervals of the dissonant key row in cents
  float knob5 = constrain(readKnobFloat(KNOB5, -knob4 / 2, knob4 / 2), -knob4 / 2, knob4 / 2); // Offset of the dissonant key row in cents
  float knob6 = constrain(readKnobFloat(KNOB6, -knob3 / 2, knob3 / 2), -knob3 / 2, knob3 / 2); // Offset of the additional consonant row in cents
  float knob7 = constrain(readKnobFloat(KNOB7, knob1, knob1 * 12), knob1, knob1 * 12); // Interval of additional consonant row in cents

  // Define the base frequency as A4 (440 Hz)
  float base_freq = 440;

  // Define the row keys
  const char* row1 = "QWERTYUIOP[]";
  const char* row2 = "ASDFGHJKL;'";
  const char* row3 = "ZXCVBNM,./";

  int idx;
  float row_offset = 0;
  int key_pos = 0;

  if ((idx = find_key_in_row(key, row1)) != -1) {
    key_pos = idx;
    row_offset = 0;
  } else if ((idx = find_key_in_row(key, row2)) != -1) {
    key_pos = idx;
    row_offset = knob5;
  } else if ((idx = find_key_in_row(key, row3)) != -1) {
    key_pos = idx;
    row_offset = knob6;
  }

  // Read vector shift knob
  float vector_shift = readKnobFloat(KNOB8, -1200, 1200);

  // Calculate the offset from the base key in cents based on the keyboard key and the knobs
  float offset = key_pos * (knob3 + knob4) + row_offset + vector_shift;

  // Add the base key and base frequency offsets to the offset
  offset += (knob2 - knob1 / 2);

  // Calculate the frequency
  float freq = freqFromCents(base_freq, offset);

  // Return the frequency
  return freq;
}

// Create a function to play a note on a given channel with a given frequency
void play_note(int channel, float freq) {
  // Set the frequency of the channel using the ledcWriteTone function
  ledcWriteTone(channel, freq);
}

// Create a function to stop playing a note on a given channel
void stop_note(int channel) {
  // Set the frequency of the channel to zero using the ledcWriteTone function
  ledcWriteTone(channel, 0);
}

// Create a function to add a note
void add_note(char key) {
  // Check if the note is already being played
  for (int i = 0; i < MAX_NOTES; i++) {
    if (active[i] && keys[i] == key) {
      return; // Already playing
    }
  }

  // Find an empty channel
  for (int i = 0; i < MAX_NOTES; i++) {
    if (!active[i]) {
      float freq = calculate_frequency(key);
      notes[i] = freq;
      keys[i] = key;
      active[i] = true;
      play_note(i, freq);
      return;
    }
  }
}

// Create a function to remove a note
void remove_note(char key) {
  for (int i = 0; i < MAX_NOTES; i++) {
    if (active[i] && keys[i] == key) {
      stop_note(i);
      active[i] = false;
      keys[i] = '\0';
      return;
    }
  }
}

// Create a setup function to initialize the esp32 and the keyboard
void setup() {
  // Initialize serial communication for debugging purposes
  Serial.begin(115200);

  // Initialize each ledc channel with a resolution of 8 bits and a frequency of 0 Hz
  for (int i = 0; i < MAX_NOTES; i++) {
    ledcSetup(i, 0, 8);
    ledcAttachPin(i + 18, i); // Attach each channel to a pin from GPIO18 to GPIO25
  }

  // Initialize the keyboard with the data and clock pins
  keyboard.begin(DATA, CLOCK);
}

// Create a loop function to read and process keyboard input
void loop() {
  // Check if there is data available from the keyboard
  if (keyboard.available()) {
    // Read and store the data from the keyboard as a char variable
    char key = keyboard.read();

    const char* all_keys = "QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM,./";

    // Check if the key is one of the valid keys for playing notes
    if (strchr(all_keys, key)) {
      add_note(key);
    } else if (key == PS2_DELETE) { // Delete key
      for (int i = 0; i < MAX_NOTES; i++) {
        if (active[i]) {
          stop_note(i);
          active[i] = false;
        }
      }
    }
  }

  // Check if any of the keys are released
  for (int i = 0; i < MAX_NOTES; i++) {
    if (active[i]) {
      char key = keys[i];
      int state = keyboard.readKeyState(key);
      if (state == 0) {
        remove_note(key);
      }
    }
  }
}
