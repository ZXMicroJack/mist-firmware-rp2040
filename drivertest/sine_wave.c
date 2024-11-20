/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <math.h>

#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "pico/bootrom.h"

#include "pico/stdlib.h"

#include "../drivers/audio_i2s.h"

#include "pico/binary_info.h"
bi_decl(bi_3pins_with_names(PICO_AUDIO_I2S_DATA_PIN, "I2S DIN", PICO_AUDIO_I2S_CLOCK_PIN_BASE, "I2S BCK", PICO_AUDIO_I2S_CLOCK_PIN_BASE+1, "I2S LRCK"));


#define SINE_WAVE_TABLE_LEN 2048
// #define SAMPLES_PER_BUFFER 256
// #define SAMPLES_PER_BUFFER PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH
#define SAMPLES_PER_BUFFER 2048

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100
#endif

static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];

struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .sample_freq = SAMPLE_RATE,
            .channel_count = 2,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
            .data_pin = PICO_AUDIO_I2S_DATA_PIN,
            .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    output_format = audio_i2s_setupx(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

int main() {
    stdio_init_all();

    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        sine_wave_table[i] = 32767 * cosf(i * 2 * (float) (M_PI / SINE_WAVE_TABLE_LEN));
    }

    struct audio_buffer_pool *ap = init_audio();
    uint32_t step = 0x200000;
    uint32_t pos = 0;
    uint32_t pos_max = 0x10000 * SINE_WAVE_TABLE_LEN;
    uint vol = 128;
    uint t;
    while (true) {
        int c = getchar_timeout_us(0);
        if (c >= 0) {
            if (c == '-' && vol) vol -= 4;
            if ((c == '=' || c == '+') && vol < 255) vol += 4;
            if (c == '[' && step > 0x10000) step -= 0x10000;
            if (c == ']' && step < (SINE_WAVE_TABLE_LEN / 16) * 0x20000) step += 0x10000;
            if (c == 'q') break;
            printf("vol = %d, step = %d      \r", vol, step >> 16);
        }
        struct audio_buffer *buffer = take_audio_buffer(ap, true);
//         uint16_t *samples = (uint16_t *) buffer->buffer->bytes;
        int16_t *samples = (int16_t *) buffer->buffer->bytes;
        for (uint i = 0; i < (buffer->max_sample_count>>1); i++) {
//             samples[i] = 0;
//             samples[i] = ((i>>6)&1) ? -256 : 256;
//             samples[i] = ((i>>6)&1) ? 0x6000 : 0xa000;
//             samples[i] = ((i>>6)&1) ? 0x00 : 0xff;
            samples[(i<<1)] = (vol * sine_wave_table[pos >> 16u]) >> 8u;
            samples[(i<<1)+1] = (vol * sine_wave_table[pos >> 16u]) >> 8u;
//             samples[i<<1)] = (vol * sine_wave_table[pos >> 16u]) >> 8u;
//             samples[(i<<1)+1] = (vol * sine_wave_table[pos >> 16u]) >> 8u;
            pos += step;
            if (pos >= pos_max) pos -= pos_max;
        }
//         buffer->sample_count = buffer->max_sample_count;
        t = buffer->sample_count = buffer->max_sample_count>>1;
        give_audio_buffer(ap, buffer);
    }
    puts("\n");
    printf("sample count = %d\n", t);
    reset_usb_boot(0, 0);
    return 0;
}
