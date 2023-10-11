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

// Define a variable to store the current pressed key and its frequency
char current_key = '\0'; // Current pressed key
float current_freq = 0;  // Current frequency

// Define a function to map a value from one range to another
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Define a function to quantize a frequency to a given granularity
float quantize(float freq, float granularity) {
  return round(freq / granularity) * granularity;
}

// Define a function to calculate the frequency of a note based on its index and offset from the base frequency and the scale interval
float calculate_frequency(int index, float offset, float base_freq, float scale_interval) {
  return base_freq * pow(scale_interval, index + offset);
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

void loop() {
  
  // Read the values from the analog knobs and map them to their corresponding ranges
  
  // Base granularity of scale: from MIN_FREQ to MAX_FREQ in Hz
  float knob1 = mapf(analogRead(KNOB1), 0, 4095, MIN_FREQ, MAX_FREQ);
  
  // Base key of the scale quantified to selected granularity: from MIN_FREQ to MAX_FREQ in Hz, quantized by knob1 value
  float knob2 = quantize(mapf(analogRead(KNOB2), 0, 4095, MIN_FREQ, MAX_FREQ), knob1);
  
  // Intervals of the consonant key row: from 1.01 to 2.00 (multiplicative factor)
  float knob3 = mapf(analogRead(KNOB3), 0, 4095, 1.01, 2.00);
  
  // Intervals of the dissonant key row: from 1.01 to 2.00 (multiplicative factor)
  float knob4 = mapf(analogRead(KNOB4), 0, 4095, 1.01, 2.00);
  
  // Offset of the dissonant key row: from -5 to 5 (additive factor)
  float knob5 = mapf(analogRead(KNOB5), 0, 4095, -5, 5);
  
  // Offset of the additional consonant row: from -5 to 5 (additive factor)
  float knob6 = mapf(analogRead(KNOB6), 0, 4095, -5, 5);
  
  // Interval of additional consonant row: from 1.01 to 2.00 (multiplicative factor)
  float knob7 = mapf(analogRead(KNOB7), 0, 4095, 1.01, 2.00);
  
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
    if (key != current_key) {
      
      // Stop playing the previous note
      stop_tone();
      
      // Update the current key
      current_key = key;
      
      // Check if the key is one of the defined keys
      for (int i = 0; i < KEYS_PER_ROW; i++) {
        if (key == CONSONANT_KEYS[i]) {
          // Calculate the frequency of the consonant note based on its index and the base frequency and interval
          current_freq = calculate_frequency(i, 0, knob2, knob3);
          break;
        }
        else if (key == DISSONANT_KEYS[i]) {
          // Calculate the frequency of the dissonant note based on its index, offset and the base frequency and interval
          current_freq = calculate_frequency(i, knob5, knob2, knob4);
          break;
        }
        else if (key == ADDITIONAL_KEYS[i]) {
          // Calculate the frequency of the additional consonant note based on its index, offset and the base frequency and interval
          current_freq = calculate_frequency(i, knob6, knob2, knob7);
          break;
        }
      }
      
      // Play the current note with the calculated frequency
      play_tone(current_freq);
      
      // Print the current key and frequency to the serial monitor for debugging
      Serial.print("Current key: "); Serial.println(current_key);
      Serial.print("Current freq: "); Serial.println(current_freq);
    }
    
    else {
      // Do nothing if the key is the same as the current key
    }
    
    delay(10); // Add a small delay to avoid bouncing keys
  }
  
}
