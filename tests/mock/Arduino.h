#ifndef ARDUINO_H
#define ARDUINO_H
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x01
#define OUTPUT 0x02
#define PULLUP 0x03
#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5
#define A6 6
#define analogRead mock_analogRead
#define random arduino_random
#define randomSeed arduino_randomSeed
#define pow(a,b) std::pow(a,b)
#ifdef __cplusplus
extern "C" {
#endif
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int mock_analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
unsigned long millis();
unsigned long micros();
void ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
void ledcAttachPin(uint8_t pin, uint8_t channel);
void ledcWriteTone(uint8_t channel, double freq);
#ifdef __cplusplus
}
#endif
long arduino_random(long howbig);
long arduino_random(long howsmall, long howbig);
void arduino_randomSeed(unsigned long seed);
#ifdef __cplusplus
template <typename T, typename L, typename H>
T constrain(T x, L a, H b) {
    if (x < (T)a) return (T)a;
    if (b < (T)x) return (T)b;
    return x;
}
using std::sqrt;
using std::round;
#include <cstring>
using std::strchr;
class Serial_ {
public:
    void begin(unsigned long baud) {}
    void print(const char* s) { std::cout << s; }
    void print(int n) { std::cout << n; }
    void print(float f) { std::cout << f; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int n) { std::cout << n << std::endl; }
    void println(float f) { std::cout << f << std::endl; }
    void println() { std::cout << std::endl; }
};
extern Serial_ Serial;
#endif
#endif
