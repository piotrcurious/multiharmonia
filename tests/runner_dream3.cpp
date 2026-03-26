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
    std::cout << "Starting Test for Dream3" << std::endl;
    analogValues[34] = 2048;
    analogValues[35] = 2048;
    analogValues[36] = 2048;
    analogValues[39] = 2048;
    analogValues[32] = 2048;
    analogValues[33] = 2048;
    analogValues[27] = 2048;
    setup();
    std::cout << "Pushing '1' (Pivotal key) to keyboard buffer" << std::endl;
    keyboardBuffer.push('1');
    keyStates['1'] = 1;
    loop();
    std::cout << "Releasing '1'" << std::endl;
    keyStates['1'] = 0;
    loop();
    return 0;
}
