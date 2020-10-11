# interceptkb

[![build](https://github.com/jeandeaual/interceptkb/workflows/build/badge.svg)](https://github.com/jeandeaual/interceptkb/actions?query=workflow%3Abuild)

Intercept events from a keyboard (like the [FS1-P foot pedal](https://store.speechrecsolutions.com/fs1-p-usb-foot-pedal-p193.aspx)) and output another key instead, using [uinput](https://www.kernel.org/doc/html/latest/input/uinput.html).

## Requirements

This only works on Linux.

## Build

* Debug:

    ```bash
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    ```

* Release:

    ```bash
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .
    ```

# Usage

```sh
./interceptkb [KEY_CODE] [INPUT_PATH]
```

For example, to output the left meta (Windows) key from the FS1-P foot pedal:

```sh
./interceptkb 125 dev/input/by-id/usb-RDing_FootSwitch1F1.-event-kbd
```

See [here](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h#L76) for a list of input event codes.
