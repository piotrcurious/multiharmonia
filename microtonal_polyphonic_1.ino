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
#define KNOB8 14 // Vector Row shift/transpose (cents)
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

// Software oscillators for each voice
Oscillator oscillators[MAX_NOTES];

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
  const char* rowP = "1234567890-=";

  int idx;
  float row_offset = 0;
  int key_pos = 0;

  if ((idx = find_key_in_row(key, row1)) != -1) {
    key_pos = idx;
    row_offset = 0; // First row is base row
  } else if ((idx = find_key_in_row(key, row2)) != -1) {
    key_pos = idx;
    row_offset = knob5; // Second row is dissonant row
  } else if ((idx = find_key_in_row(key, row3)) != -1) {
    key_pos = idx;
    row_offset = knob6; // Third row is additional row
  } else if ((idx = find_key_in_row(key, rowP)) != -1) {
    key_pos = idx;
    // Pivotal row: geometric mean of row1 and row2 logic
    // We'll calculate the base and dissonant frequencies and take sqrt
    float f1 = freqFromCents(base_freq, key_pos * knob3);
    float f2 = freqFromCents(base_freq, key_pos * knob4 + knob5);
    float fp = sqrt(f1 * f2);
    // Convert back to cents relative to base_freq
    row_offset = 1200.0f * log2(fp / base_freq) - (key_pos * knob3); // Relative adjustment
  }

  // Read vector shift knob
  float vector_shift = readKnobFloat(KNOB8, -1200, 1200);

  // Calculate the offset from the base key in cents based on the keyboard key and the knobs
  // Each key in a row is spaced by knob3 or knob4 depending on row, or just a constant scale
  // Based on README: Consonant row is consonant tones, dissonant row is dissonant tones.
  // We'll use knob3 for consonant spacing and knob4 for dissonant spacing if in those rows?
  // Or more simply, key_pos defines the horizontal position in the scale.
  float spacing = ((idx = find_key_in_row(key, row2)) != -1) ? knob4 : knob3;
  float offset = key_pos * spacing + row_offset + vector_shift;

  // Add the base key and base frequency offsets to the offset
  offset += (knob2 - knob1 / 2);

  // Calculate the frequency
  float freq = freqFromCents(base_freq, offset);

  // Return the frequency
  return freq;
}

// Create a function to play a note
void play_note(int channel, float freq) {
  oscillators[channel].setFrequency(freq);
}

// Create a function to stop playing a note
void stop_note(int channel) {
  oscillators[channel].setFrequency(0);
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

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Timer interrupt for audio generation
void IRAM_ATTR onTimer() {
  float mix = 0;
  int activeVoices = 0;

  for (int i = 0; i < MAX_NOTES; i++) {
    if (oscillators[i].frequency > 0) {
      mix += oscillators[i].nextSample();
      activeVoices++;
    }
  }

  if (activeVoices > 0) {
    mix /= activeVoices; // Normalize
  }

  // Output to DAC (GPIO 25)
  // Scale -1.0..1.0 to 0..255
  int val = (int)((mix + 1.0f) * 127.5f);
  dacWrite(25, val);
}

// Create a setup function to initialize the esp32 and the keyboard
void setup() {
  // Initialize serial communication for debugging purposes
  Serial.begin(115200);
  Serial.println("Microtonal Polyphonic Synthesizer Starting (Software Mixed)...");

  // Initialize the keyboard with the data and clock pins
  keyboard.begin(DATA, CLOCK);

  // Set up Timer for audio (SAMPLE_RATE)
  // Timer 0, divider 80 (1us per tick if 80MHz)
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  // Alarm every (1,000,000 / SAMPLE_RATE) us
  timerAlarmWrite(timer, 1000000 / (int)SAMPLE_RATE, true);
  timerAlarmEnable(timer);
}

// Note tracker to handle state
NoteTracker tracker;

// Create a loop function to read and process keyboard input
void loop() {
  // Check if there is data available from the keyboard
  if (keyboard.available()) {
    // Read and store the data from the keyboard as a char variable
    int raw_key = keyboard.read();
    char key = (char)(raw_key & 0xFF);

    // We'll treat every read as a press event for standard ASCII-only PS2 libraries
    // unless we have specific break detection logic.
    // Since we need to know when a key is released, we'll use tracker.states
    // updated by keyboard.readKeyState below.

    const char* all_keys = "QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM,./1234567890-=";

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

  // Update tracker states and handle releases
  for (int i = 0; i < MAX_NOTES; i++) {
    if (active[i]) {
      char key = keys[i];
      // If library supports readKeyState, we use it to detect release
      if (keyboard.readKeyState(key) == 0) {
        remove_note(key);
      }
    }
  }
}
