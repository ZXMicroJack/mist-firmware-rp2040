#include <stdio.h>
#include <stdint.h>
#include <pico/time.h>
#include "wtsynth.h"
#include "picosynthversion.h"

#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "drivers/audio_i2s.h"
#include "drivers/pins.h"

// #define DEBUG
#include "drivers/debug.h"

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

static struct audio_buffer_pool *init_audio() {

    static audio_format_t audio_format = {
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .sample_freq = SAMPLE_RATE,
            .channel_count = 2,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 4
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
            .data_pin = PICO_AUDIO_I2S_DATA_PIN,
            .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
            .dma_channel = 0,
            .pio_sm = AUDIO_I2S_SM,
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

static int synth_status = 0;
static uint8_t pending_state = 1;
static uint8_t running_state = 1;
static struct audio_buffer_pool *ap;
void picosynth_Init() {
  int ret = wtsynth_Init();
  debug(("picosynth_Init: luts returns %d\n", ret));
  if (ret < 0) {
    LUTS_POS = (uint8_t *)RP2M_LUTS2_POS;
    ret = wtsynth_Init();
    debug(("picosynth_Init: luts2 returns %d\n", ret));
  }
  synth_status = ret;
  ap = init_audio();
  debug(("picosynth_Init: all ok\n"));
}

void picosynth_Loop() {
  /* grab audio packets if synth is started */
  if (!synth_status) {
    struct audio_buffer *buffer = take_audio_buffer(ap, false);
    if (buffer) {
      int16_t *samples = (int16_t *) buffer->buffer->bytes;
      if (running_state) {
        wtsynth_GetAudioPacket(samples);
        for (int i=0; i<SAMPLES_PER_BUFFER; i++)
          samples[i] ^= 0x8000;
      } else {
        for (int i=0; i<SAMPLES_PER_BUFFER; i++)
          samples[i] = 0;
      }
      buffer->sample_count = buffer->max_sample_count>>1;
      give_audio_buffer(ap, buffer);
      nrbuffs++;
    }

    if (pending_state != running_state) {
      running_state = pending_state;
      wtsynth_Suspend(!running_state);
    }
  }
}

void picosynth_Suspend(uint8_t state) {
  pending_state = !state;
  debug(("picosynth_Suspend: state %d\n", state));
}

void wtsynth_ActiveState(uint8_t active) {
}

void wtsynth_SetBar(uint8_t chan, uint8_t level) {
}

