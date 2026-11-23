#include "platform.h"
#include <stdint.h>
#include "audio.h"

typedef struct {
    uint8_t  half_period; /* samples per half wave: 8000 / (2 * Hz) */
    uint16_t length;      /* samples */
} SoundDef;

static const SoundDef k_sounds[] = {
    { 0,  0   }, /* SOUND_NONE */
    { 10, 800 }, /* SFX_PISTOL */
    { 5,  400 }  /* SFX_PICKUP */
};
#define SOUND_COUNT (uint8_t)(sizeof k_sounds / sizeof k_sounds[0])

typedef struct {
    uint16_t remaining;
    uint8_t half_period;
    uint8_t count;
    uint8_t high;
} Channel;

/* volatile: written by main code while read by ISR audio thread */
static volatile Channel s_channels[AUDIO_CHANNELS];
static uint8_t s_next_channel;

void sound_play(uint8_t sound_id) {
    /* The oldest sound gets cut off first */
    platform_play_sound(s_next_channel, sound_id);
    s_next_channel = (uint8_t)((s_next_channel + 1u) % AUDIO_CHANNELS); /* round-robin */
}

void core_audio_start(uint8_t channel, uint8_t sound_id) {
    volatile Channel *c;

    if (channel >= AUDIO_CHANNELS) return;
    if (sound_id >= SOUND_COUNT) sound_id = SOUND_NONE;

    c = &s_channels[channel];
    c->remaining   = k_sounds[sound_id].length;
    c->half_period = k_sounds[sound_id].half_period;
    c->count       = 0;
    c->high        = 0;
}

uint8_t core_audio_next_sample() {
    int16_t mix = 0;
    uint8_t i;

    for (i = 0; i < AUDIO_CHANNELS; i++) {
        volatile Channel *c = &s_channels[i];
        if (c->remaining == 0u) continue;
        c->remaining--;
        if (++c->count >= c->half_period) {
            c->count = 0;
            c->high  = (uint8_t)!c->high;
        }
        mix = (int16_t)(mix + (c->high ? 40 : -40)); /* 3x40 stays in range */
    }
    return (uint8_t)(mix + 128);
}
