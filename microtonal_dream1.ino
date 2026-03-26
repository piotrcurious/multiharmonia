// Include the libraries for esp32 and ps2 keyboard
#include <Arduino.h>
#include <PS2Keyboard.h>
#include "SynthUtils.h"

// Define the pins for the keyboard and the knobs
#define DATA_PIN 16
#define CLOCK_PIN 17
#define KNOB1_PIN 34
#define KNOB2_PIN 35
#define KNOB3_PIN 36
#define KNOB4_PIN 39
#define KNOB5_PIN 32
#define KNOB6_PIN 33
#define KNOB7_PIN 27
#define KNOB8_PIN 14

// Create an object for the keyboard
PS2Keyboard keyboard;

// Define some constants for the musical instrument
#define MAX_NOTES 128 // The maximum number of notes in a scale
#define MAX_FREQ 8000 // The maximum frequency in Hz
#define MIN_FREQ 20 // The minimum frequency in Hz
#define BASE_FREQ 440 // The base frequency in Hz for A4 note
#define BASE_NOTE 69 // The MIDI note number for A4 note

// Define some variables for the musical instrument
int notes[MAX_NOTES]; // An array to store the frequencies of the notes in a scale
int baseGranularity; // The base granularity of the scale in cents (1/100 of a semitone)
int baseKey; // The base key of the scale quantified to the selected granularity
int consonantIntervals; // The intervals of the consonant key row in cents
int dissonantIntervals; // The intervals of the dissonant key row in cents
int dissonantOffset; // The offset of the dissonant key row in cents

// A function to generate a scale based on the knob values
void generateScale() {
  // Read the knob values and constrain them to reasonable ranges
  baseGranularity = constrain(readKnobInt(KNOB1_PIN, 1, 100), 1, 100); // From 1 cent to 100 cents
  baseKey = constrain(readKnobInt(KNOB2_PIN, -1200, 1200), -1200, 1200); // From -12 semitones to +12 semitones

  // Quantize baseKey by baseGranularity
  baseKey = (baseKey / baseGranularity) * baseGranularity;

  consonantIntervals = constrain(readKnobInt(KNOB3_PIN, 100, 1200), 100, 1200); // From 1 semitone to 12 semitones
  dissonantIntervals = constrain(readKnobInt(KNOB4_PIN, -1200, -100), -1200, -100); // From -12 semitones to -1 semitone
  dissonantOffset = constrain(readKnobInt(KNOB5_PIN, -600, 600), -600, 600); // From -6 semitones to +6 semitones
  int vectorShift = readKnobInt(KNOB8_PIN, -1200, 1200);

  // Loop through the notes and calculate their frequencies based on the intervals and offset
  for (int i = 0; i < MAX_NOTES; i++) {
    int interval; // The interval in cents from the base key
    if (i % 2 == 0) { // If it is an even note, use the consonant intervals
      interval = baseKey + (i / 2) * consonantIntervals + vectorShift;
    } else { // If it is an odd note, use the dissonant intervals and offset
      interval = baseKey + ((i - 1) / 2) * dissonantIntervals + dissonantOffset + vectorShift;
    }
    // Calculate the frequency using the formula f = f0 * (2 ^ (n / 1200))
    float frequency = freqFromCents(BASE_FREQ, (float)interval);
    // Constrain the frequency to the minimum and maximum values
    frequency = constrain(frequency, (float)MIN_FREQ, (float)MAX_FREQ);
    // Store the frequency in the notes array
    notes[i] = (int)frequency;
    }
}

// A function to play a note based on a key press
void playNote(char key) {
   // Map the key to a note index using the ASCII code
   // Map the key to a note index using the ASCII code
   int noteIndex = (unsigned char)key - 32;
   // Constrain the note index to the valid range
   noteIndex = constrain(noteIndex, 0, MAX_NOTES - 1);
   // Get the frequency of the note from the notes array
   int frequency = notes[noteIndex];
   // Set the PWM frequency of pin 25 to the note frequency
   ledcWriteTone(0, frequency);
}

// A function to stop playing a note
void stopNote() {
  // Set the PWM frequency of pin 25 to zero
  ledcWriteTone(0, 0);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Microtonal Dream1 Synthesizer Starting...");

  // Initialize the keyboard
  keyboard.begin(DATA_PIN, CLOCK_PIN);
  // Initialize the PWM channel 0 on pin 25 with 8-bit resolution
  ledcSetup(0, 5000, 8);
  ledcAttachPin(25, 0);
}

// Current active key in monophonic mode
char activeKey = '\0';

void loop() {
  // Generate a scale based on the knob values
  generateScale();
  
  // Check if a key is available
  if (keyboard.available()) {
    activeKey = keyboard.read();
    playNote(activeKey);
  }

  // Check for release if library supports it
  if (activeKey != '\0' && keyboard.readKeyState(activeKey) == 0) {
    stopNote();
    activeKey = '\0';
  }
}
