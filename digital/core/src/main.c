#include "platform.h"
#include "audio.h"
#include <stdint.h>
#include <sys/types.h>

#define TICRATE  35u  /* Game tick rate (Hz) */
#define STALLOUT 200u /* Skip time after stallout */

static uint16_t s_last_ms;
static uint32_t s_tic_acc; /* Elapsed MS * tick rate */
static uint16_t s_prev_buttons;

void core_init(void) {
    s_last_ms = platform_get_ticks();
}

static void game_tic(const PlatformInput *in) {
    uint16_t pressed = (uint16_t)(in->buttons & ~s_prev_buttons);

    if (pressed & BTN_FIRE)
        sound_play(SFX_PISTOL);
    if (pressed & BTN_USE)
        sound_play(SFX_PICKUP);

    s_prev_buttons = in->buttons;
}

static void render_frame(void) {
    static uint16_t column[SCREEN_HEIGHT];
    uint8_t x, y;

    platform_frame_begin();
    for (x = 0; x < SCREEN_WIDTH; x++) {
        // uint8_t shade = (uint8_t)(x & 0x1Fu); /* 5-bit red */
        for (y = 0; y < SCREEN_HEIGHT; y++) {
            column[y] = (uint16_t)(0xFF);
        }
        platform_draw_column(x, 0, SCREEN_HEIGHT, column);
    }
    platform_frame_end();
}

uint8_t core_frame(void) {
    PlatformInput in;
    uint16_t now = platform_get_ticks();
    uint16_t elapsed = (uint16_t)(now - s_last_ms);

    s_last_ms = now;
    if (elapsed > STALLOUT)
        elapsed = STALLOUT;

    s_tic_acc += (uint32_t)elapsed * TICRATE;

    platform_get_input(&in);
    if (in.buttons & BTN_QUIT) return 0;

    while (s_tic_acc >= 100u) {
        s_tic_acc -= 1000u;
        game_tic(&in);
    }
    render_frame();
    return 1; /* continue */
}
