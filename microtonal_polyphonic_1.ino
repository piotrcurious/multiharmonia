// Include the libraries for esp32 and ps2 keyboard
#include <Arduino.h>
#include <PS2Keyboard.h>

// Define the pins for the analog knobs and the keyboard data and clock
#define KNOB1 A0 // Base granularity of scale
#define KNOB2 A1 // Base key of the scale quantified to selected granularity
#define KNOB3 A2 // Intervals of the consonant key row
#define KNOB4 A3 // Intervals of the dissonant key row
#define KNOB5 A4 // Offset of the dissonant key row
#define KNOB6 A5 // Offset of the additional consonant row
#define KNOB7 A6 // Interval of additional consonant row
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
int notes[MAX_NOTES];

// Create an array to store the corresponding keyboard keys for each note
char keys[MAX_NOTES];

// Create a variable to store the number of notes being played
int note_count = 0;

// Create a function to map a value from one range to another
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Create a function to read an analog knob value and map it to a desired range
float read_knob(int pin, float min, float max) {
  int raw = analogRead(pin); // Read the raw value from 0 to 4095
  return mapf(raw, 0, 4095, min, max); // Map it to the desired range
}

// Create a function to calculate the frequency of a note based on the knob values and the keyboard key
float calculate_frequency(char key) {
  // Read the knob values and constrain them to reasonable ranges
  float knob1 = constrain(read_knob(KNOB1, 1, 100), 1, 100); // Base granularity of scale in cents (1/100 of a semitone)
  float knob2 = constrain(read_knob(KNOB2, 0, knob1), 0, knob1); // Base key of the scale quantified to selected granularity in cents
  float knob3 = constrain(read_knob(KNOB3, knob1, knob1 * 12), knob1, knob1 * 12); // Intervals of the consonant key row in cents
  float knob4 = constrain(read_knob(KNOB4, knob1, knob1 * 12), knob1, knob1 * 12); // Intervals of the dissonant key row in cents
  float knob5 = constrain(read_knob(KNOB5, -knob4 / 2, knob4 / 2), -knob4 / 2, knob4 / 2); // Offset of the dissonant key row in cents
  float knob6 = constrain(read_knob(KNOB6, -knob3 / 2, knob3 / 2), -knob3 / 2, knob3 / 2); // Offset of the additional consonant row in cents
  float knob7 = constrain(read_knob(KNOB7, knob1, knob1 * 12), knob1, knob1 * 12); // Interval of additional consonant row in cents

  // Define the base frequency as A4 (440 Hz)
  float base_freq = 440;

  // Define the base key as Q on the keyboard
  char base_key = 'Q';

  // Calculate the offset from the base key in cents based on the keyboard key and the knobs
  float offset = (key - base_key) * (knob3 + knob4) + (key >= 'A' ? knob5 : (key >= 'Z' ? knob6 : 0));

  // Add the base key and base frequency offsets to the offset
  offset += (knob2 - knob1 / 2);

  // Calculate the frequency by multiplying the base frequency by two raised to the power of offset divided by twelve hundred
  float freq = base_freq * pow(2, offset / 1200);

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

// Create a function to add a note to the notes array
void add_note(char key) {
  // Check if the note is already in the array
  for (int i = 0; i < note_count; i++) {
    if (keys[i] == key) {
      return; // Do nothing if the note is already in the array
    }
  }

  // Check if there is space in the array
  if (note_count < MAX_NOTES) {
    // Calculate the frequency of the note based on the key and the knobs
    float freq = calculate_frequency(key);

    // Add the note and the key to the array
    notes[note_count] = freq;
    keys[note_count] = key;

    // Play the note on the corresponding channel
    play_note(note_count, freq);

    // Increment the note count
    note_count++;
  }
}

// Create a function to remove a note from the notes array
void remove_note(char key) {
  // Find the index of the note in the array
  int index = -1;
  for (int i = 0; i < note_count; i++) {
    if (keys[i] == key) {
      index = i; // Store the index of the note
      break;
    }
  }

  // Check if the note was found in the array
  if (index != -1) {
    // Stop playing the note on the corresponding channel
    stop_note(index);

    // Shift the notes and keys in the array to fill the gap
    for (int i = index; i < note_count - 1; i++) {
      notes[i] = notes[i + 1];
      keys[i] = keys[i + 1];
      play_note(i, notes[i]); // Update the channel frequency
    }

    // Decrement the note count
    note_count--;
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

    // Check if the key is one of the valid keys for playing notes
    if (key >= 'Q' && key <= 'P') { // Consonant key row
      add_note(key); // Add the note to the array and play it
    } else if (key >= 'A' && key <= 'L') { // Dissonant key row
      add_note(key); // Add the note to the array and play it
    } else if (key >= 'Z' && key <= 'M') { // Additional consonant row
      add_note(key); // Add the note to the array and play it
    } else if (key == PS2_DELETE) { // Delete key
      for (int i = 0; i < note_count; i++) {
        stop_note(i); // Stop playing all notes on all channels
      }
      note_count = 0; // Reset the note count and clear the array
    }
  }

  // Check if any of the keys are released by reading them again
  for (int i = 0; i < note_count; i++) {
    char key = keys[i]; // Get the key from the array

    // Read and store the state of the key using the keyboard.readKeyState function
    int state = keyboard.readKeyState(key);

    // Check if the state is zero, meaning that the key is released
    if (state == 0) {
      remove_note(key); // Remove the note from the array and stop playing it
    }
  }
}
