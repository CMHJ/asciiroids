#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include <unistd.h>
void usleep(unsigned int useconds); // get usleep with c99 enabled

#include "types.h"

static const usize MAX_PATH = 255;

static void handle_keyboard_events(const i8 fd) {
    bool running = true;
    struct input_event event;
    while (running) {
        i8 n = read(fd, &event, sizeof(event));
        if (n < (i8)sizeof(event)) {
            // no event to read
            continue;
        }

        if (event.type != EV_KEY) {
            continue;
        }

        // exit when player presses 'q'
        if (event.code == KEY_Q) {
            running = false;
        }

        const char* key_state_text;
        switch (event.value) {
        case 0: {
            key_state_text = "Released";
            break;
        }
        case 1: {
            key_state_text = "Pressed";
            break;
        }
        case 2: {
            key_state_text = "Held";
            break;
        }
        default: {
            key_state_text = "Unknown";
            break;
        }
        }

        printf("Key %d: %s\n", event.code, key_state_text);
    }

    close(fd);
}

static u8 count_input_devices(void) {
    DIR* dir = opendir("/dev/input/");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    u8 count = 0;
    for (struct dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
        const bool found_event_device = (strncmp(entry->d_name, "event", 5) == 0);
        if (found_event_device) {
            count += 1;
        }
    }

    closedir(dir);

    return count;
}

// returns file descriptor for keyboard input device
static i8 detect_keyboard(void) {
    i8 keyboard_fd = -1;

    // count input devices
    u8 input_devices_count = count_input_devices();

    // open all input devices
    static const usize max_input_devices = 100;
    i8 input_device_file_descriptors[max_input_devices];
    char input_device_path[MAX_PATH];
    for (usize i = 0; i < input_devices_count; ++i) {
        if (i > max_input_devices) {
            printf("Reached maximum number of input devices\n");
            exit(1);
        }

        snprintf(input_device_path, MAX_PATH, "/dev/input/event%ld", i);
        input_device_file_descriptors[i] = open(input_device_path, O_RDONLY | O_NONBLOCK);
    }

    // iterate through until enter key press is detected
    bool keyboard_found = false;
    while (keyboard_found == false) {
        struct input_event event;
        for (usize i = 0; i < input_devices_count; ++i) {
            i8 input_device_fd = input_device_file_descriptors[i];

            i8 bytes_read = read(input_device_fd, &event, sizeof(event));
            if (bytes_read < (i8)sizeof(event)) {
                // no event to read
                continue;
            }

            if (event.type != EV_KEY) {
                continue;
            }

            if (event.code == KEY_ENTER) {
                keyboard_found = true;
                keyboard_fd = input_device_fd;
                printf("Found keyboard\n");
            }
        }

        // limit polling to a reasonable rate
        usleep(1);
    }

    // close all input devices that aren't the detected one
    // TODO(CMHJ): implement this

    return keyboard_fd;
}
