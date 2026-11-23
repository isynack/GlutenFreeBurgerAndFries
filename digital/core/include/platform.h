/*
 * Interface between game core and hardware.
 *
 * Core calls the PLATFORM functions below to interact with the platform.
 * Implementation in platform/sdl/ for the PC and in platform/pic24 for the PIC24.
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

// Constants
#define SCREEN_WIDTH 160u
#define SCREEN_HEIGHT 128u

#define AUDIO_SAMPLE_RATE  8000u
#define AUDIO_CHANNELS 3u

#define STICK_MAX 1023u

// Button bits; platform does debouncing
#define BTN_FIRE (1u << 0)
#define BTN_USE (1u << 1)
#define BTN_STRAFE (1u << 2)
#define BTN_QUIT (1u << 15) // PC only for window close

typedef struct {
    uint16_t buttons;
    uint16_t stick_x;
    uint16_t stick_y;
} PlatformInput;

// Bring up display, input, tick timer, and output audio
void platform_init(void);

// Called once before the first column of a frame.
// PIC: Wait for the previous frame to complete.
// SDL: Potentially implement slow-mode timing.
void platform_frame_begin(void);

// Draw one virtical strip of pixels at screen column x, rows y..y+height-1.
// The 3D view uses y = 0, height = VIEW_HEIGHT while the status bar redraws for y >= VIEW_HEIGHT.
// Each platform copies the pixels before returning so the core may reuse its buffer.
//
// Pixels are native-endian RGB565.
void platform_draw_column(uint8_t x, uint8_t y, uint8_t height, const uint16_t* pixels);

// Called once after the last column of a frame.
// PIC: Wait for the frame to complete.
// SDL:
void platform_frame_end(void);

// Returns the number of ticks since the platform was initialized. Wrapping every 65.5s
// Always compare with unsigned subtraction
uint16_t platform_get_ticks(void);

// Returns the current input state.
//   Fills *input with the latest debounced input state.
// SDL: Also window events and BTN_QUIT
void platform_get_input(PlatformInput *input);

// Start or stop a sound.
//   Starts sound_id on given chanel from 0..AUDIO_CHANNELS-1
//   sound_id SOUND_NONE stops the channel
// The platform blocks its audio interrupt or callback
void platform_play_sound(uint8_t channel, uint8_t sound_id);

// Called once by backend main() after platform_init()
void core_init(void);

// Called once per frame by platform main()
//   Runs at 35Hz
// Return 0 for game exit (BTN_QUIT)
uint8_t core_frame(void);

// Audio mixer: returns next audio sample to play.
//   PIC: Called once per 8kHz TMR3 ISR, result written to OC1RS
//   SDL: Called in loop from audio stream callback to fill buffer
//   Runs only in an interrupt context
uint8_t core_audio_next_sample(void);

// Set channel state. Call only from inside platform_play_sound()
//   while ISR or callback is blocked.
void core_audio_start(uint8_t channel, uint8_t sound_id);

#endif /* PLATFORM_H */
