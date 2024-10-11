#include "print.h"

static void print_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t* string, usize n) {
    // check error cases
    if (x < 0) {
        return;
    }
    if (buffer->width <= x) {
        return;
    }
    if (y < 0) {
        return;
    }
    if (buffer->height <= y) {
        return;
    }

    // check for buffer overrun and clip string if so
    if (x + n > buffer->width) {
        n = buffer->width - x;
    }
    assert(0 <= n);

    const usize index = (y * buffer->width) + x;

    // check write doesn't overrun end of buffer
    assert((index + n - 1) < buffer->size);

    wchar_t* dst = &buffer->data[index];
    // TODO(CMHJ): replace this with own implementation that doesn't have null terminator.
    // although this already seems to be the case?
    // as last wchar in buffer can be written to by string that doesn't start on last char.
    wcsncpy(dst, string, n);
}

static void printwc_xy(screen_buffer* buffer, u32 x, u32 y, wchar_t c) {
    // TODO(CMHJ): add wrapping to this function
    static wchar_t buf[2] = {0};
    buf[0] = c;
    print_xy(buffer, x, y, buf, 1);
}
