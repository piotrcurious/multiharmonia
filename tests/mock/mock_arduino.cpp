#include "Arduino.h"
#include <cstdio>
#include <vector>
#include <queue>
#include <map>
int analogValues[128] = {0};
double toneFrequencies[16] = {0};
std::queue<char> keyboardBuffer;
std::map<char, int> keyStates;
extern "C" {
void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
int digitalRead(uint8_t pin) { return 0; }
int mock_analogRead(uint8_t pin) { return analogValues[pin]; }
void analogWrite(uint8_t pin, int val) {}
void delay(uint32_t ms) {}
void delayMicroseconds(uint32_t us) {}
unsigned long millis() { return 0; }
unsigned long micros() { return 0; }
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits) {}
void ledcAttachPin(uint8_t pin, uint8_t channel) {}
void ledcWriteTone(uint8_t channel, double freq) {
    if (channel < 16) {
        toneFrequencies[channel] = freq;
        printf("Tone set on channel %d: %f Hz\n", channel, freq);
    }
}
void dacWrite(uint8_t pin, uint8_t value) {
    // printf("DAC set on pin %d: %d\n", pin, value);
}
}

hw_timer_t * timerBegin(uint8_t num, uint16_t divider, bool countUp) { return NULL; }
void timerAttachInterrupt(hw_timer_t *timer, void (*fn)(void), bool edge) {}
void timerAlarmWrite(hw_timer_t *timer, uint64_t interruptAt, bool autoreload) {}
void timerAlarmEnable(hw_timer_t *timer) {}
void arduino_randomSeed(unsigned long seed) { std::srand(seed); }
long arduino_random(long howbig) { return (howbig > 0) ? std::rand() % howbig : 0; }
long arduino_random(long howsmall, long howbig) {
    if (howsmall >= howbig) return howsmall;
    return howsmall + (std::rand() % (howbig - howsmall));
}
Serial_ Serial;
#include "PS2Keyboard.h"
void PS2Keyboard::begin(uint8_t data_pin, uint8_t irq_pin) {}
unsigned int PS2Keyboard::available() { return keyboardBuffer.size(); }
char PS2Keyboard::read() {
    if (keyboardBuffer.empty()) return 0;
    char c = keyboardBuffer.front();
    keyboardBuffer.pop();
    return c;
}
int PS2Keyboard::readKeyState(char key) { return keyStates[key]; }
#include "driver/dac.h"
void dac_output_enable(dac_channel_t channel) {}
void dac_output_disable(dac_channel_t channel) {}
void dac_frequency_set(dac_channel_t channel, uint32_t freq) {
    printf("DAC Tone set on channel %d: %u Hz\n", (int)channel, freq);
}
