// Include the libraries for esp32 and ps2 keyboard
#include <Arduino.h>
#include <PS2Keyboard.h>

// Define the pins for the keyboard and the knobs
#define DATA_PIN 2
#define CLOCK_PIN 3
#define KNOB1_PIN 34
#define KNOB2_PIN 35
#define KNOB3_PIN 36
#define KNOB4_PIN 39
#define KNOB5_PIN 32

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

// A function to map a value from one range to another
int mapValue(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

// A function to read the value of a knob and map it to a range
int readKnob(int pin, int min, int max) {
  int value = analogRead(pin); // Read the analog value from 0 to 4095
  return mapValue(value, 0, 4095, min, max); // Map it to the desired range
}

// A function to generate a scale based on the knob values
void generateScale() {
  // Read the knob values and constrain them to reasonable ranges
  baseGranularity = constrain(readKnob(KNOB1_PIN, 1, 100), 1, 100); // From 1 cent to 100 cents
  baseKey = constrain(readKnob(KNOB2_PIN, -1200, 1200), -1200, 1200); // From -12 semitones to +12 semitones
  consonantIntervals = constrain(readKnob(KNOB3_PIN, 100, 1200), 100, 1200); // From 1 semitone to 12 semitones
  dissonantIntervals = constrain(readKnob(KNOB4_PIN, -1200, -100), -1200, -100); // From -12 semitones to -1 semitone
  dissonantOffset = constrain(readKnob(KNOB5_PIN, -600, 600), -600, 600); // From -6 semitones to +6 semitones

  // Calculate the number of notes in the scale based on the base granularity
  int numNotes = MAX_NOTES / baseGranularity;

  // Loop through the notes and calculate their frequencies based on the intervals and offset
  for (int i = 0; i < numNotes; i++) {
    int interval; // The interval in cents from the base key
    if (i % 2 == 0) { // If it is an even note, use the consonant intervals
      interval = baseKey + (i / 2) * consonantIntervals;
    } else { // If it is an odd note, use the dissonant intervals and offset
      interval = baseKey + ((i - 1) / 2) * dissonantIntervals + dissonantOffset;
    }
    // Calculate the frequency using the formula f = f0 * (2 ^ (n / 1200))
    int frequency = BASE_FREQ * pow(2.0, interval / 1200.0);
    // Constrain the frequency to the minimum and maximum values
    frequency = constrain(frequency, MIN_FREQ, MAX_FREQ);
    // Store the frequency in the notes array
    notes[i] = frequency;
    }
}

// A function to play a note based on a key press
void playNote(char key) {
   // Map the key to a note index using the ASCII code
   int noteIndex = key - 32; // Subtract 32 to start from the space key
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
  // Initialize the keyboard
  keyboard.begin(DATA_PIN, CLOCK_PIN);
  // Initialize the PWM channel 0 on pin 25 with 8-bit resolution
  ledcSetup(0, 5000, 8);
  ledcAttachPin(25, 0);
}

void loop() {
  // Generate a scale based on the knob values
  generateScale();
  
  // Check if a key is pressed
  if (keyboard.available()) {
    // Read the key
    char key = keyboard.read();
    // Play the corresponding note
    playNote(key);
  } else {
    // Stop playing the note
    stopNote();
  }
}
