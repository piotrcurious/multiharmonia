#include "Arduino.h"
#include <queue>
#include <map>
#include <iostream>
extern int analogValues[128];
extern std::queue<char> keyboardBuffer;
extern std::map<char, int> keyStates;
void setup();
void loop();
int main() {
    std::cout << "Starting Test for Dream2" << std::endl;
    analogValues[A0] = 2048;
    analogValues[A1] = 2048;
    analogValues[A2] = 2048;
    analogValues[A3] = 2048;
    analogValues[A4] = 2048;
    analogValues[A5] = 2048;
    analogValues[A6] = 2048;
    setup();
    std::cout << "Pushing 'Q' to keyboard buffer" << std::endl;
    keyboardBuffer.push('Q');
    loop();
    std::cout << "Looping without key" << std::endl;
    loop();
    return 0;
}
