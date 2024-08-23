#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "types.h"

u8 count_input_devices(void) {
    DIR* dir = opendir("/dev/input/");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    u8 count = 0;
    for (struct dirent* entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
        printf("%s\n", entry->d_name);
        const bool found_event_device = (strncmp(entry->d_name, "event", 5) == 0);
        if (found_event_device) {
            count += 1;
        }
    }

    closedir(dir);

    return count;
}

void detect_keyboard(void) {
    // count input devices
    u8 input_devices = count_input_devices();
    printf("Input devices detected: %d\n", input_devices);

    // open all input devices
    // iterate through until enter key press is detected
    // close all input devices that aren't the detected one
    // return
}
