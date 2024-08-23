// #include <stdio.h>
// #include <stdlib.h>
// #include <wchar.h>
// #include <locale.h>

#include "input_keyboard.c"
#include "terminal.c"
#include "types.h"

#define WIDTH 80
#define HEIGHT 24

int32_t main(void) {
    // setlocale(LC_CTYPE, "");
    // wchar_t star = 0x2605;
    // wprintf(L"%lc\n", star);

    set_terminal_echo(false);

    printf("Press the enter key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = detect_keyboard();
    handle_keyboard_events(keyboard_fd);

    // TODO(CMHJ): clear stdin of all keyboard inputs after pressing 'q'

    set_terminal_echo(true);

    return EXIT_SUCCESS;
}
