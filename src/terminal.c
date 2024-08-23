#include <stdbool.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static void set_terminal_echo(const bool enable) {
    struct termios tty_settings;
    tcgetattr(STDIN_FILENO, &tty_settings);

    if (enable) {
        tty_settings.c_lflag |= ECHO;
    } else {
        tty_settings.c_lflag &= ~(ECHO);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tty_settings);
}
