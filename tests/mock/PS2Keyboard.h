#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H
#include <cstdint>
#define PS2_KC_NONE 0
#define PS2_DELETE 127
class PS2Keyboard {
public:
    PS2Keyboard() {}
    void begin(uint8_t data_pin, uint8_t irq_pin);
    unsigned int available();
    char read();
    int readKeyState(char key);
};
#endif
