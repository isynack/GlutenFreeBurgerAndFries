#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

enum {
    SOUND_NONE,
    SFX_PISTOL,
    SFX_PICKUP
};

void sound_play(uint8_t sound_id);

#endif
