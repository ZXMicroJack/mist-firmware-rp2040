#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include "wtsynth.h"
#include "picosynthversion.h"

#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "drivers/audio_i2s.h"
#include "drivers/pins.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 44100
#endif
#ifndef SAMPLE_LEN
#define SAMPLE_LEN 1024
#endif

#define SAMPLES_PER_BUFFER SAMPLE_LEN*2

uint8_t *LUTS_POS = (uint8_t *)RP2M_LUTS_POS;
uint8_t *SOUNDFONT_POS = (uint8_t *)RP2M_SOUNDFONT_POS;
uint8_t *SOUNDFONT2_POS = (uint8_t *)RP2M_SOUNDFONT2_POS;
int nrbuffs = 0;

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
            .pio_sm = 2,
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


void audio_core() {
  struct audio_buffer_pool *ap = init_audio();
  
  for(;;) {
    // read and process audio data
    struct audio_buffer *buffer = take_audio_buffer(ap, true);
    int16_t *samples = (int16_t *) buffer->buffer->bytes;
    wtsynth_GetAudioPacket(samples);
    buffer->sample_count = buffer->max_sample_count>>1;
    give_audio_buffer(ap, buffer);
    nrbuffs++;
  }
}

void wtsynth_ActiveState(uint8_t active) {
}

void wtsynth_SetBar(uint8_t chan, uint8_t level) {
}

void picosynth_Init() {
  int ret = wtsynth_Init();
  if (ret < 0) {
    LUTS_POS = (uint8_t *)RP2M_LUTS2_POS;
    ret = wtsynth_Init();
  }

  if (!ret) {
    multicore_reset_core1();
    multicore_launch_core1(audio_core);
  }
}
