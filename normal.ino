
// Include the libraries for PS2 keyboard and ESP32 DAC
#include <PS2Keyboard.h>
#include <driver/dac.h>

// Define the pins for the keyboard and the knobs
#define DATA_PIN 16
#define CLOCK_PIN 17
#define KNOB1_PIN 34
#define KNOB2_PIN 35
#define KNOB3_PIN 36
#define KNOB4_PIN 39

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

// A function to map a value from one range to another
int mapValue(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

// A function to read the analog knobs and update the scale and note parameters
void readKnobs() {
  // Read the knob values
  knob1Value = analogRead(KNOB1_PIN);
  knob2Value = analogRead(KNOB2_PIN);
  knob3Value = analogRead(KNOB3_PIN);
  knob4Value = analogRead(KNOB4_PIN);

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
  int dissonantOffset = random(0, MAX_SCALE); // Pick a random offset for the dissonant row
  for (int i = 0; i < MAX_SCALE; i++) {
    if (i < dissonantSize) {
      dissonantRow[i] = scale[(i + dissonantOffset) % MAX_SCALE]; // Use modular arithmetic to wrap around the scale
    } else {
      dissonantRow[i] = -1; // Mark the unused intervals as -1
    }
  }
}

// A function to read the keyboard and update the note state
void readKeyboard() {
  // Check if a key is available
  if (keyboard.available()) {
    // Read the key
    key = keyboard.read();

    // Check if the key is valid
    if (key != PS2_KC_NONE) {
      // Set the key pressed flag to true
      keyPressed = true;

      // Map the key to a note using the consonant and dissonant rows
      switch (key) {
        case 'q':
          note = baseNote + consonantRow[0];
          break;
        case 'w':
          note = baseNote + consonantRow[1];
          break;
        case 'e':
          note = baseNote + consonantRow[2];
          break;
        case 'r':
          note = baseNote + consonantRow[3];
          break;
        case 't':
          note = baseNote + consonantRow[4];
          break;
        case 'y':
          note = baseNote + consonantRow[5];
          break;
        case 'u':
          note = baseNote + consonantRow[6];
          break;
        case 'i':
          note = baseNote + consonantRow[7];
          break;
        case 'o':
          note = baseNote + consonantRow[8];
          break;
        case 'p':
          note = baseNote + consonantRow[9];
          break;
        case '[':
          note = baseNote + consonantRow[10];
          break;
        case ']':
          note = baseNote + consonantRow[11];
          break;
        case 'a':
          note = baseNote + dissonantOffset + dissonantRow[0];
          break;
        case 's':
          note = baseNote + dissonantOffset + dissonantRow[1];
          break;
        case 'd':
          note = baseNote + dissonantOffset + dissonantRow[2];
          break;
        case 'f':
          note = baseNote + dissonantOffset + dissonantRow[3];
          break;
        case 'g':
          note = baseNote + dissonantOffset + dissonantRow[4];
          break;
        case 'h':
          note = baseNote + dissonantOffset + dissonantRow[5];
          break;
        case 'j':
          note = baseNote + dissonantOffset + dissonantRow[6];
          break;
        case 'k':
          note = baseNote + dissonantOffset + dissonantRow[7];
          break;
        case 'l':
          note = baseNote + dissonantOffset + dissonantRow[8];
          break;
        case ';':
          note = baseNote + dissonantOffset + dissonantRow[9];
          break;
        case '\'':
          note = baseNote + dissonantOffset + dissonantRow[10];
          break;
        default:
          // If the key is not mapped, set the key pressed flag to false and return
          keyPressed = false;
          return;
      }

      // Clamp the note to the valid range
      if (note < 0) {
        note = 0;
      } else if (note > MAX_NOTE) {
        note = MAX_NOTE;
      }

      // Calculate the frequency of the note using the formula f = 440 * 2^((n-69)/12)
      noteFreq = A4_FREQ * pow(2, (note - A4_NOTE) / 12.0);
    }
  } else {
    // If no key is available, set the key pressed flag to false
    keyPressed = false;
  }
}

// A function to play a tone using the ESP32 DAC
void playTone(int freq) {
  // Set the DAC output frequency
  dac_frequency_set(DAC_CHANNEL_1, freq);

  // Enable the DAC output
  dac_output_enable(DAC_CHANNEL_1);
}

// A function to stop the tone
void stopTone() {
  // Disable the DAC output
  dac_output_disable(DAC_CHANNEL_1);
}

// The setup function runs once when the board is powered on or reset
void setup() {
  // Initialize the keyboard
  keyboard.begin(DATA_PIN, CLOCK_PIN);

  // Initialize the DAC
  dac_output_enable(DAC_CHANNEL_1);

  // Initialize the random seed
  randomSeed(analogRead(0));
}

// The loop function runs repeatedly after the setup function is completed
void loop() {
  // Read the knobs and update the scale and note parameters
  readKnobs();

  // Read the keyboard and update the note state
  readKeyboard();

  // Check if a key is pressed
  if (keyPressed) {
    // Play the note using the DAC
    playTone(noteFreq);
  } else {
    // Stop the tone
    stopTone();
  }
}

