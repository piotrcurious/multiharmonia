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
    std::cout << "Starting Test for Dream1" << std::endl;
    analogValues[34] = 2048;
    analogValues[35] = 2048;
    analogValues[36] = 2048;
    analogValues[39] = 2048;
    analogValues[32] = 2048;
    setup();
    loop();
    std::cout << "Pushing ' ' (space) to keyboard buffer" << std::endl;
    keyboardBuffer.push(' ');
    loop();
    std::cout << "Looping without key" << std::endl;
    loop();
    return 0;
}
