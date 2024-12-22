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

static void keyboard_state_update(game_state* game, const i8 fd) {
    controller_state* p1_cont = &game->controllers[0];
    controller_state* p2_cont = &game->controllers[1];
    controller_state* p3_cont = &game->controllers[2];
    controller_state* p4_cont = &game->controllers[3];
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
            // player 1 binds
            case KEY_Q: {
                if (event.value == RELEASED) {
                    p1_cont->quit = false;
                } else if (event.value == PRESSED) {
                    p1_cont->quit = true;
                }
                break;
            }
            case KEY_UP: {
                if (event.value == RELEASED) {
                    p1_cont->up = false;
                } else if (event.value == PRESSED) {
                    p1_cont->up = true;
                }
                break;
            }
            case KEY_LEFT: {
                if (event.value == RELEASED) {
                    p1_cont->left = false;
                } else if (event.value == PRESSED) {
                    p1_cont->left = true;
                }
                break;
            }
            case KEY_RIGHT: {
                if (event.value == RELEASED) {
                    p1_cont->right = false;
                } else if (event.value == PRESSED) {
                    p1_cont->right = true;
                }
                break;
            }
            case KEY_DOWN: {
                if (event.value == RELEASED) {
                    p1_cont->down = false;
                } else if (event.value == PRESSED) {
                    p1_cont->down = true;
                }
                break;
            }
            case KEY_RIGHTCTRL: {
                if (event.value == RELEASED) {
                    p1_cont->shoot = false;
                } else if (event.value == PRESSED) {
                    p1_cont->shoot = true;
                }
                break;
            }

            // player 2 binds
            case KEY_I: {
                if (event.value == RELEASED) {
                    p2_cont->up = false;
                } else if (event.value == PRESSED) {
                    p2_cont->up = true;
                }
                break;
            }
            case KEY_J: {
                if (event.value == RELEASED) {
                    p2_cont->left = false;
                } else if (event.value == PRESSED) {
                    p2_cont->left = true;
                }
                break;
            }
            case KEY_L: {
                if (event.value == RELEASED) {
                    p2_cont->right = false;
                } else if (event.value == PRESSED) {
                    p2_cont->right = true;
                }
                break;
            }
            case KEY_K: {
                if (event.value == RELEASED) {
                    p2_cont->down = false;
                } else if (event.value == PRESSED) {
                    p2_cont->down = true;
                }
                break;
            }
            case KEY_M: {
                if (event.value == RELEASED) {
                    p2_cont->shoot = false;
                } else if (event.value == PRESSED) {
                    p2_cont->shoot = true;
                }
                break;
            }

            // player 3 binds
            case KEY_T: {
                if (event.value == RELEASED) {
                    p3_cont->up = false;
                } else if (event.value == PRESSED) {
                    p3_cont->up = true;
                }
                break;
            }
            case KEY_F: {
                if (event.value == RELEASED) {
                    p3_cont->left = false;
                } else if (event.value == PRESSED) {
                    p3_cont->left = true;
                }
                break;
            }
            case KEY_H: {
                if (event.value == RELEASED) {
                    p3_cont->right = false;
                } else if (event.value == PRESSED) {
                    p3_cont->right = true;
                }
                break;
            }
            case KEY_G: {
                if (event.value == RELEASED) {
                    p3_cont->down = false;
                } else if (event.value == PRESSED) {
                    p3_cont->down = true;
                }
                break;
            }
            case KEY_V: {
                if (event.value == RELEASED) {
                    p3_cont->shoot = false;
                } else if (event.value == PRESSED) {
                    p3_cont->shoot = true;
                }
                break;
            }

            // player 4 binds
            case KEY_W: {
                if (event.value == RELEASED) {
                    p4_cont->up = false;
                } else if (event.value == PRESSED) {
                    p4_cont->up = true;
                }
                break;
            }
            case KEY_A: {
                if (event.value == RELEASED) {
                    p4_cont->left = false;
                } else if (event.value == PRESSED) {
                    p4_cont->left = true;
                }
                break;
            }
            case KEY_D: {
                if (event.value == RELEASED) {
                    p4_cont->right = false;
                } else if (event.value == PRESSED) {
                    p4_cont->right = true;
                }
                break;
            }
            case KEY_S: {
                if (event.value == RELEASED) {
                    p4_cont->down = false;
                } else if (event.value == PRESSED) {
                    p4_cont->down = true;
                }
                break;
            }
            case KEY_Z: {
                if (event.value == RELEASED) {
                    p4_cont->shoot = false;
                } else if (event.value == PRESSED) {
                    p4_cont->shoot = true;
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

static void update_inputs(game_state* game, i8* controller_fds) {
    const i8 keyboard_fd = controller_fds[0];
    keyboard_state_update(game, keyboard_fd);
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
