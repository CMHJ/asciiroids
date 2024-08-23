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
    detect_keyboard();

    return EXIT_SUCCESS;
}
