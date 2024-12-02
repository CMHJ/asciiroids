# Asciiroids

Requires root to detect the keyboard and read the keyboard inputs directly as opposed to using stdin which has limitations when used as a game controller.

# Setup

Requires adding user to the `input` group so that they can access the input devices at `/dev/input`.

```sh
sudo usermod -aG input "$USER"
# reboot or logout to refresh
```

# Building

```sh
./build.sh  # debug
# or
./build.sh release  # release
```

# Todo

- [x] Investigate adding a `game` user that is a member of `input` group such that playing the game and getting inputs doesn't require root.
