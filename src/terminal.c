#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static void terminal_set_settings(const bool enable) {
    struct termios tty_settings;
    if (tcgetattr(STDIN_FILENO, &tty_settings) != 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    if (enable) {
        // disable echo so that key presses aren't displayed.
        // disable canonical mode so that key inputs can be processed immediately,
        // this is useful for flushing stdin on exit
        tty_settings.c_lflag &= ~(ICANON | ECHO);
    } else {
        tty_settings.c_lflag |= (ICANON | ECHO);
    }

    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty_settings) != 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/* This is required, otherwise the shell prompt displayed after exiting the game will contain all the keyboard inputs
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

static void terminal_setup(void) {
    terminal_set_settings(true);
}

static void terminal_teardown(void) {
    terminal_flush_stdin();
    terminal_set_settings(false);
}
