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
    std::cout << "Starting Test for Polyphonic" << std::endl;
    analogValues[34] = 2048;
    analogValues[35] = 2048;
    setup();
    std::cout << "Pushing 'Q' then 'W' to keyboard buffer" << std::endl;
    keyboardBuffer.push('Q');
    keyStates['Q'] = 1;
    loop();
    keyboardBuffer.push('W');
    keyStates['W'] = 1;
    loop();
    std::cout << "Releasing 'Q'" << std::endl;
    keyStates['Q'] = 0;
    loop();
    std::cout << "Pushing 'A' to keyboard buffer" << std::endl;
    keyboardBuffer.push('A');
    keyStates['A'] = 1;
    loop();
    std::cout << "Pushing 'Z' to keyboard buffer" << std::endl;
    keyboardBuffer.push('Z');
    keyStates['Z'] = 1;
    loop();
    return 0;
}
