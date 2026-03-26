#ifndef DAC_H
#define DAC_H
#include <cstdint>
typedef enum {
    DAC_CHANNEL_1 = 1,
    DAC_CHANNEL_2 = 2,
} dac_channel_t;
void dac_output_enable(dac_channel_t channel);
void dac_output_disable(dac_channel_t channel);
void dac_frequency_set(dac_channel_t channel, uint32_t freq);
#endif
