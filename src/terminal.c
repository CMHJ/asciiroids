#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "types.h"

static void terminal_set_settings(const bool enable) {
    struct termios tty_settings;
    if (tcgetattr(STDIN_FILENO, &tty_settings) != 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    if (enable) {
        // disable echo so that key presses aren't displayed.
        // disable canonical mode so that key inputs can be processed immediately,
        // this is useful for flushing stdin on exit.
        tty_settings.c_lflag &= ~(ICANON | ECHO);
    } else {
        tty_settings.c_lflag |= (ICANON | ECHO);
    }

    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty_settings) != 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/*
 * This is required, otherwise the shell prompt displayed after exiting the game will contain all the keyboard inputs
 * pressed while playing the game.
 */
static void terminal_flush_stdin(void) {
    // this should clear all inputs and catch the 'q' pressed from trying to exit the game
    for (char c = -1; read(STDIN_FILENO, &c, 1) == 1;) {
        if (c == 'q') {
            break;
        }
    }
}

void terminal_clear_screen() {
    wprintf(L"\033[H\033[J");
}

/* Switch to an alternate screen buffer. */
void terminal_save_buffer(void) {
    wprintf(L"\033[?1049h");
    fflush(stdout);
}

/* Switch back to the main screen buffer. */
void terminal_restore_buffer(void) {
    wprintf(L"\033[?1049l");
    fflush(stdout);
}

void terminal_reset_cursor_position(void) {
    wprintf(L"\033[H");
    fflush(stdout);
}

void terminal_cursor_hide(void) {
    wprintf(L"\033[?25l");
    fflush(stdout);
}

void terminal_cursor_show(void) {
    wprintf(L"\033[?25h");
    fflush(stdout);
}

static void terminal_setup(void) {
    terminal_set_settings(true);
    terminal_save_buffer();
    terminal_clear_screen();
    terminal_reset_cursor_position();
    terminal_cursor_hide();
}

static void terminal_teardown(void) {
    terminal_flush_stdin();
    terminal_set_settings(false);
    terminal_cursor_show();
    terminal_restore_buffer();
}

void buffer_render(screen_buffer* buffer) {
    // TODO(CMHJ): remove these debug lines
    // terminal_clear_screen();

    terminal_reset_cursor_position();

    for (usize row = 0; row < buffer->height; ++row) {
        for (usize col = 0; col < buffer->width; ++col) {
            wprintf(L"%lc", buffer->data[(buffer->width * row) + col]);
        }
        wprintf(L"\n");
    }
}

void buffer_set(screen_buffer* buffer, const wchar_t c) {
    for (usize row = 0; row < buffer->height; ++row) {
        for (usize col = 0; col < buffer->width; ++col) {
            buffer->data[(buffer->width * row) + col] = c;
        }
    }
}
