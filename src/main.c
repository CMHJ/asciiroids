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

    terminal_setup();

    printf("Press the enter key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = keyboard_detect();

    u32 i = 0;
    for (bool running = true; running; ++i) {
        static controller_state keyboard_controller_state = {0};
        keyboard_state_update(keyboard_fd, &keyboard_controller_state);

        if (keyboard_controller_state.quit) {
            running = false;
        }

        printf("%d: Up: %d, Left: %d, Right: %d, Shoot: %d\n", i, keyboard_controller_state.up,
               keyboard_controller_state.left, keyboard_controller_state.right, keyboard_controller_state.shoot);

        sleep(1);
    }

    close(keyboard_fd);

    terminal_teardown();

    return EXIT_SUCCESS;
}
