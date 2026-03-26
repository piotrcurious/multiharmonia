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
    std::cout << "Starting Test for Normal" << std::endl;
    analogValues[34] = 2048;
    analogValues[35] = 2048;
    setup();
    std::cout << "Pushing 'q' to keyboard buffer" << std::endl;
    keyboardBuffer.push('q');
    keyStates['q'] = 1;
    loop();
    std::cout << "Releasing 'q'" << std::endl;
    keyStates['q'] = 0;
    loop();

    std::cout << "Looping without key" << std::endl;
    loop();
    return 0;
}
