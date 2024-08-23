// #include <stdio.h>
// #include <stdlib.h>
// #include <wchar.h>
// #include <locale.h>

#include "types.h"
#include "input_keyboard.c"

#define WIDTH 80
#define HEIGHT 24

int32_t main(void)
{
    // setlocale(LC_CTYPE, "");
    // wchar_t star = 0x2605;
    // wprintf(L"%lc\n", star);

    // list_input_devices_and_find_keyboard();

    printf("Press the enter key...\n");
    // funnily enough this seems to detect the enter key event from starting the program
    const i8 keyboard_fd = detect_keyboard();
    handle_keyboard_events(keyboard_fd);

    return EXIT_SUCCESS;
}
