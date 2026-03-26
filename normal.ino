
// Include the libraries for PS2 keyboard and ESP32 DAC
#include <PS2Keyboard.h>
#include "SynthUtils.h"

// Define the pins for the keyboard and the knobs
#define DATA_PIN 16
#define CLOCK_PIN 17
#define KNOB1_PIN 34
#define KNOB2_PIN 35
#define KNOB3_PIN 36
#define KNOB4_PIN 39
#define KNOB8_PIN 14

// Create an object for the keyboard
PS2Keyboard keyboard;

// Define some constants for the musical scales and notes
#define MAX_SCALE 12 // The maximum number of notes in a scale
#define MAX_NOTE 127 // The maximum MIDI note number
#define A4_NOTE 69 // The MIDI note number for A4 (440 Hz)
#define A4_FREQ 440 // The frequency of A4 in Hz

// Define some arrays for storing the scale and note information
int scale[MAX_SCALE]; // The scale intervals in semitones
int baseNote; // The base note of the scale
int consonantRow[MAX_SCALE]; // The consonant row intervals in semitones
int dissonantRow[MAX_SCALE]; // The dissonant row intervals in semitones
int dissonantOffset; // The offset of the dissonant row in semitones

// Define some variables for storing the knob values
int knob1Value; // The value of knob 1 (0-4095)
int knob2Value; // The value of knob 2 (0-4095)
int knob3Value; // The value of knob 3 (0-4095)
int knob4Value; // The value of knob 4 (0-4095)

// Define some variables for storing the keyboard state
char key; // The last pressed key
bool keyPressed; // Whether a key is pressed or not

// Define some variables for storing the note state
int note; // The current note to play
int noteFreq; // The frequency of the current note in Hz

// Define static members of Oscillator
float Oscillator::sineTable[SINE_TABLE_SIZE];
bool Oscillator::tableInitialized = false;

// Variables to store previous knob values for change detection
int prevKnob1 = -1, prevKnob2 = -1, prevKnob3 = -1, prevKnob4 = -1, prevKnob8 = -1;
#define KNOB_THRESHOLD 50

// A function to read the analog knobs and update the scale and note parameters
void readKnobs() {
  // Read the knob values
  knob1Value = analogRead(KNOB1_PIN);
  knob2Value = analogRead(KNOB2_PIN);
  knob3Value = analogRead(KNOB3_PIN);
  knob4Value = analogRead(KNOB4_PIN);
  int knob8Value = analogRead(KNOB8_PIN);

  // Only regenerate scale if knobs have moved significantly
  if (!knobMoved(knob1Value, prevKnob1, KNOB_THRESHOLD) &&
      !knobMoved(knob2Value, prevKnob2, KNOB_THRESHOLD) &&
      !knobMoved(knob3Value, prevKnob3, KNOB_THRESHOLD) &&
      !knobMoved(knob4Value, prevKnob4, KNOB_THRESHOLD) &&
      !knobMoved(knob8Value, prevKnob8, KNOB_THRESHOLD)) {
    return;
  }

  prevKnob1 = knob1Value;
  prevKnob2 = knob2Value;
  prevKnob3 = knob3Value;
  prevKnob4 = knob4Value;
  prevKnob8 = knob8Value;

  // Map the knob values to the scale and note parameters
  baseNote = mapValue(knob1Value, 0, 4095, 0, MAX_NOTE); // Map knob 1 to the base note (0-127)
  int scaleSize = mapValue(knob2Value, 0, 4095, 1, MAX_SCALE); // Map knob 2 to the scale size (1-12)
  int scaleStep = mapValue(knob3Value, 0, 4095, 1, MAX_SCALE); // Map knob 3 to the scale step (1-12)
  
  // Generate the scale intervals using the scale size and step
  for (int i = 0; i < MAX_SCALE; i++) {
    if (i < scaleSize) {
      scale[i] = (i * scaleStep) % MAX_SCALE; // Use modular arithmetic to wrap around the scale
    } else {
      scale[i] = -1; // Mark the unused intervals as -1
    }
  }

  int consonantSize = mapValue(knob4Value, 0, 4095, 1, MAX_SCALE); // Map knob 4 to the consonant row size (1-12)
  
  // Generate the consonant row intervals using the consonant size and a random offset
  int consonantOffset = random(0, MAX_SCALE); // Pick a random offset for the consonant row
  for (int i = 0; i < MAX_SCALE; i++) {
    if (i < consonantSize) {
      consonantRow[i] = scale[(i + consonantOffset) % MAX_SCALE]; // Use modular arithmetic to wrap around the scale
    } else {
      consonantRow[i] = -1; // Mark the unused intervals as -1
    }
  }

    // Generate the dissonant row intervals using the consonant size and a random offset
  int dissonantSize = MAX_SCALE - consonantSize; // The dissonant row size is the complement of the consonant row size
  dissonantOffset = random(0, MAX_SCALE); // Pick a random offset for the dissonant row
  for (int i = 0; i < MAX_SCALE; i++) {
    if (i < dissonantSize) {
      dissonantRow[i] = scale[(i + dissonantOffset) % MAX_SCALE]; // Use modular arithmetic to wrap around the scale
    } else {
      dissonantRow[i] = -1; // Mark the unused intervals as -1
    }
  }
}

// Monophonic voice management
MonoVoice voice;
float current_freq = 0;

float calc_current_freq(char key) {
  int local_note = -1;
  switch (key) {
    case 'q': local_note = baseNote + consonantRow[0]; break;
    case 'w': local_note = baseNote + consonantRow[1]; break;
    case 'e': local_note = baseNote + consonantRow[2]; break;
    case 'r': local_note = baseNote + consonantRow[3]; break;
    case 't': local_note = baseNote + consonantRow[4]; break;
    case 'y': local_note = baseNote + consonantRow[5]; break;
    case 'u': local_note = baseNote + consonantRow[6]; break;
    case 'i': local_note = baseNote + consonantRow[7]; break;
    case 'o': local_note = baseNote + consonantRow[8]; break;
    case 'p': local_note = baseNote + consonantRow[9]; break;
    case '[': local_note = baseNote + consonantRow[10]; break;
    case ']': local_note = baseNote + consonantRow[11]; break;
    case 'a': local_note = baseNote + dissonantOffset + dissonantRow[0]; break;
    case 's': local_note = baseNote + dissonantOffset + dissonantRow[1]; break;
    case 'd': local_note = baseNote + dissonantOffset + dissonantRow[2]; break;
    case 'f': local_note = baseNote + dissonantOffset + dissonantRow[3]; break;
    case 'g': local_note = baseNote + dissonantOffset + dissonantRow[4]; break;
    case 'h': local_note = baseNote + dissonantOffset + dissonantRow[5]; break;
    case 'j': local_note = baseNote + dissonantOffset + dissonantRow[6]; break;
    case 'k': local_note = baseNote + dissonantOffset + dissonantRow[7]; break;
    case 'l': local_note = baseNote + dissonantOffset + dissonantRow[8]; break;
    case ';': local_note = baseNote + dissonantOffset + dissonantRow[9]; break;
    case '\'': local_note = baseNote + dissonantOffset + dissonantRow[10]; break;
    default: return 0;
  }
  local_note = constrain(local_note, 0, MAX_NOTE);
  float vectorShift = mapf(analogRead(KNOB8_PIN), 0, 4095, -1200, 1200);
  return freqFromCents(A4_FREQ, (local_note - A4_NOTE) * 100.0f + vectorShift);
}

// A function to play a tone using the ESP32 LEDC
void playTone(int freq) {
  ledcWriteTone(0, freq);
}

// A function to stop the tone
void stopTone() {
  ledcWriteTone(0, 0);
}

// The setup function runs once when the board is powered on or reset
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Normal Scale Synthesizer Starting...");

  // Initialize the keyboard
  keyboard.begin(DATA_PIN, CLOCK_PIN);

  // Initialize the LEDC
  ledcSetup(0, 5000, 8);
  ledcAttachPin(25, 0);

  // Initialize the random seed
  randomSeed(analogRead(0));
}

// The loop function runs repeatedly after the setup function is completed
void loop() {
  // Read the knobs and update the scale and note parameters
  readKnobs();

  // Update voice
  voice.update(keyboard, calc_current_freq);

  // Play tone if frequency changed
  if (voice.oscillator.frequency != current_freq) {
    current_freq = voice.oscillator.frequency;
    playTone(current_freq);
  }
}

