#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
void usleep(unsigned int useconds);  // get usleep with c99 enabled

#include "constants.h"
#include "types.h"

enum button_state { RELEASED, PRESSED, LAST_HELD_REPEATING };

static void keyboard_state_update(const i8 fd, controller_state* ctrlr_state) {
    struct input_event event;
    while (true) {
        i8 n = read(fd, &event, sizeof(event));
        if (n < (i8)sizeof(event)) {
            // no event to read, done updating state
            break;
        }

        // we don't care about anything that isn't a key press
        if (event.type != EV_KEY) {
            continue;
        }

        switch (event.code) {
            case KEY_Q: {
                if (event.value == RELEASED) {
                    ctrlr_state->quit = false;
                } else if (event.value == PRESSED) {
                    ctrlr_state->quit = true;
                }
                break;
            }
            case KEY_UP: {
                if (event.value == RELEASED) {
                    ctrlr_state->up = false;
                } else if (event.value == PRESSED) {
                    ctrlr_state->up = true;
                }
                break;
            }
            case KEY_LEFT: {
                if (event.value == RELEASED) {
                    ctrlr_state->left = false;
                } else if (event.value == PRESSED) {
                    ctrlr_state->left = true;
                }
                break;
            }
            case KEY_RIGHT: {
                if (event.value == RELEASED) {
                    ctrlr_state->right = false;
                } else if (event.value == PRESSED) {
                    ctrlr_state->right = true;
                }
                break;
            }
            case KEY_DOWN: {
                if (event.value == RELEASED) {
                    ctrlr_state->down = false;
                } else if (event.value == PRESSED) {
                    ctrlr_state->down = true;
                }
                break;
            }
            case KEY_SPACE: {
                if (event.value == RELEASED) {
                    ctrlr_state->shoot = false;
                } else if (event.value == PRESSED) {
                    ctrlr_state->shoot = true;
                }
                break;
            }
            default: {
                // do nothing
                break;
            }
        }
    }
}

static void update_inputs(game_state* state, i8* controller_fds) {
    const i8 keyboard_fd = controller_fds[0];
    controller_state* keyboard_controller_state = &state->controllers[0];
    keyboard_state_update(keyboard_fd, keyboard_controller_state);
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

/*
 * Returns file descriptor for keyboard input device.
 * Caller's responsibility to close the file descriptor.
 */
static i8 keyboard_detect(void) {
    i8 keyboard_fd = -1;

    // count input devices
    u8 input_devices_count = count_input_devices();

    // open all input devices
    static const usize max_input_devices = 100;
    i8 input_device_file_descriptors[max_input_devices];
    char input_device_path[MAX_PATH];
    for (usize i = 0; i < input_devices_count; ++i) {
        if (i > max_input_devices) {
            fprintf(stderr, "Reached maximum number of input devices\n");
            exit(EXIT_FAILURE);
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
            }
        }

        // TODO: change to nanosleep
        // limit polling to a reasonable rate
        usleep(1);
    }

    // close all input devices that aren't the detected one
    for (usize i = 0; i < input_devices_count; ++i) {
        const i8 input_device_fd = input_device_file_descriptors[i];

        if (input_device_fd != keyboard_fd) {
            close(input_device_fd);
        }
    }

    return keyboard_fd;
}
