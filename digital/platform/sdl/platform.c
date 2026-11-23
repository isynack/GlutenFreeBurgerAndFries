#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <stdlib.h>

#include "platform.h"

#define WINDOW_SCALE 4

static SDL_Window       *s_window;
static SDL_Renderer     *s_renderer;
static SDL_Texture      *s_texture;
static SDL_AudioStream  *s_audio;
static Uint64           s_start_ms;

/**
 * Emulates the LCD's memory
 */
static uint16_t s_framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];

static void fail(const char *what)
{
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error: %s\n", what);
    exit(1);
}

/**
 * SDL calls this on audio thread whenever the stream needs more data
 * - PIC: 8kHz TMR3 ISR
 * - SDL: stream lock during execution, relied on by platform_play_sound
 */
static void SDLCALL audio_callback(void *userdata, SDL_AudioStream *stream,
    int additional_amount, int total_amount) {
    Uint8 buf[256];
    (void)userdata;
    (void)total_amount;

    while (additional_amount > 0) {
        int n = additional_amount < (int)sizeof(buf) ? additional_amount : (int)sizeof(buf);
        for (int i = 0; i < n; i++) {
            buf[i] = core_audio_next_sample();
        }
        SDL_PutAudioStreamData(stream, buf, n);
        additional_amount -= n;
    }
}

void platform_init(void) {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        fail("SDL_Init");
    }

    if (!SDL_CreateWindowAndRenderer("gluten free burger and fries", SCREEN_WIDTH * WINDOW_SCALE,
        SCREEN_HEIGHT * WINDOW_SCALE, 0, &s_window, &s_renderer)) {
        fail("SDL_CreateWindowAndRenderer");
    }

    SDL_SetRenderVSync(s_renderer, 1); // Cap frame rate to monitor refresh rate
    SDL_SetRenderLogicalPresentation(s_renderer, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    s_texture = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!s_texture) {
        fail("SDL_CreateTexture");
    }

    SDL_SetTextureScaleMode(s_texture, SDL_SCALEMODE_NEAREST); // Crisp pixels

    const SDL_AudioSpec spec = {
        .freq = AUDIO_SAMPLE_RATE,
        .format = SDL_AUDIO_U8,
        .channels = 1,
    };
    s_audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_callback, NULL);
    if (!s_audio) {
        fail("SDL_OpenAudioDeviceStream");
    }
    SDL_ResumeAudioStreamDevice(s_audio);

    s_start_ms = SDL_GetTicks();
}

/**
 * Nothing to prepare on SDL
 */
void platform_frame_begin(void) {

}

void platform_draw_column(uint8_t x, uint8_t y, uint8_t height, const uint16_t *pixels) {
    for (uint8_t i = 0; i < height; i++) {
        s_framebuf[y + i][x] = pixels[i];
    }
}

/**
 * Display the frame buffer on the screen.
 */
void platform_frame_end(void) {
    SDL_UpdateTexture(s_texture, NULL, s_framebuf, SCREEN_WIDTH * sizeof(uint16_t));
    SDL_RenderClear(s_renderer);
    SDL_RenderTexture(s_renderer, s_texture, NULL, NULL);
    SDL_RenderPresent(s_renderer);
}

/**
 * Returns the number of ticks since the platform was initialized.
 * SDL would give 64-bit ms, but we truncate to emulate the PIC's wrap behavior.
 */
uint16_t platform_get_ticks(void) {
    return (uint16_t)(SDL_GetTicks() - s_start_ms);
}

void platform_get_input(PlatformInput *input) {
    SDL_Event e;
    const bool *key;

    input->buttons = 0;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            input->buttons |= BTN_QUIT;
            return;
        }
    }

    key = SDL_GetKeyboardState(NULL);
    input->stick_x = key[SDL_SCANCODE_LEFT] ? 0u :
        key[SDL_SCANCODE_RIGHT] ? STICK_MAX : 512u;
    input->stick_y = key[SDL_SCANCODE_DOWN] ? 0u
        : key[SDL_SCANCODE_UP] ? STICK_MAX : 512u;

    if (key[SDL_SCANCODE_LCTRL])
        input->buttons |= BTN_FIRE;
    if (key[SDL_SCANCODE_SPACE])
        input->buttons |= BTN_USE;
    if (key[SDL_SCANCODE_LALT])
        input->buttons |= BTN_STRAFE;
    if (key[SDL_SCANCODE_ESCAPE])
        input->buttons |= BTN_QUIT;
}

void platform_play_sound(uint8_t channel, uint8_t sound_id) {
    SDL_LockAudioStream(s_audio);
    core_audio_start(channel, sound_id);
    SDL_UnlockAudioStream(s_audio);
}
