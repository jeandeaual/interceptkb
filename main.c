#include <errno.h>
#include <fcntl.h> /* For O_RDONLY */
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h> /* For open(), close() */

#define VALUE_RELEASED   0
#define VALUE_PRESSED    1
#define VALUE_AUTOREPEAT 2

#define UINPUT_PATH "/dev/uinput"

/* The FS1-P outputs the B key */
#define DEFAULT_IN_KEY KEY_B
/* Output escape key presses by default */
#define DEFAULT_OUT_KEY KEY_ESC
/* Default device path of the FS1-P on Linux */
#define DEFAULT_DEVICE_PATH \
    "/dev/input/by-id/usb-RDing_FootSwitch1F1.-event-kbd"

void usage(char *name);
void setup_uinput(int uinput_fd, int out_key);

int main(int argc, char *argv[])
{
    /* Structure to hold event info */
    struct input_event event;
    char *device_path = DEFAULT_DEVICE_PATH;
    char name[256] = "Unknown";
    ssize_t rd = 0;
    int out_key = DEFAULT_OUT_KEY;
    int input_fd = 0;
    int uinput_fd = 0;
    int exit_code = EXIT_SUCCESS;

    /* Check the arguments */
    if (argc > 3) {
        fprintf(stderr, "Too many arguments\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc >= 2) {
        /* The key to output */
        /* See include/linux/input-event-codes.h for a list of valid codes */
        if (sscanf (argv[1], "%i", &out_key) != 1) {
            fprintf(stderr, "Error: argument 1 is not an integer: %s\n",
                    argv[1]);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        if (out_key < 0) {
            fprintf(stderr, "Error: KEY_CODE is too small: %i\n", out_key);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        if (out_key > 255) {
            fprintf(stderr, "Error: KEY_CODE is too large: %i\n", out_key);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (argc == 3) {
        device_path = argv[2];
    }

    /* Open the file for reading */
    if ((input_fd = open(device_path, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open device \"%s\": %s\n",
                device_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ioctl(input_fd, EVIOCGNAME(sizeof(name)), name) < 0) {
        fprintf(stderr, "Couldn't read name of device \"%s\": %s\n",
                device_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Reading from %s\n", name);

    if (ioctl(input_fd, EVIOCGRAB, 1) != 0) {
        fprintf(stderr, "Unable to get exclusive access to \"%s\": %s\n",
                device_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((uinput_fd = open(UINPUT_PATH, O_WRONLY)) == -1) {
        fprintf(stderr, "Unable to open device \"%s\": %s\n",
                UINPUT_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }
    setup_uinput(uinput_fd, out_key);

    printf("Receiving keyboard presses and outputting key code %i\n", out_key);

    while (1) {
        memset((void*)&event, 0, sizeof(event));

        rd = read(input_fd, &event, sizeof(event));
        if (rd == -1) {
            fprintf(stderr, "Unable to read from \"%s\": %s\n",
                    name, strerror(errno));
            exit_code = EXIT_FAILURE;
            break;
        } else if (rd < ((ssize_t) sizeof(event))) {
            fprintf(stderr, "Not enough data read from \"%s\": %zd\n",
                    name, rd);
            exit_code = EXIT_FAILURE;
            break;
        }

#ifndef NDEBUG
        printf("Event type: %d, code: %d, value: %d\n",
               event.type, event.code, event.value);
#endif

        /*
         * The input event order for the FS1-P is as follows:
         *
         * Pressed:
         *
         * event: type=4 (EV_MSC), code=4  (MSC_SCAN),   value=458757
         * event: type=1 (EV_KEY), code=48 (KEY_B),      value=1 (PRESSED)
         * event: type=0 (EV_SYN), code=0  (SYN_REPORT), value=0
         *
         * Released:
         *
         * event: type=4 (EV_MSC), code=4  (MSC_SCAN),   value=458757
         * event: type=1 (EV_KEY), code=48 (KEY_B),      value=0 (RELEASED)
         * event: type=0 (EV_SYN), code=0  (SYN_REPORT), value=0
         *
         * Autorepeat:
         *
         * event: type=1 (EV_KEY), code=48 (KEY_B),      value=2 (AUTOREPEAT)
         * event: type=0 (EV_SYN), code=0  (SYN_REPORT), value=1
         */

        /* Read the event */
        if (event.type == EV_KEY) {
            /* Keyboard event */
            if (event.code == DEFAULT_IN_KEY) {
                /*
                 * For EV_KEY, the event value will be 0 for release,
                 * 1 for keypress and 2 for autorepeat.
                 */

#ifndef NDEBUG
                if (event.value == VALUE_PRESSED) {
                    printf("Pressed\n");
                } else if (event.value == VALUE_RELEASED) {
                    printf("Released\n");
                } else if (event.value == VALUE_AUTOREPEAT) {
                    printf("Autorepeat\n");
                }
#endif

                /* Change the key */
                event.code = out_key;
            }
        }

        /* Send the modified keypress */
        if (write(uinput_fd, &event, sizeof(event)) < 0) {
            fprintf(stderr, "Unable to write to \"%s\": %s\n",
                    UINPUT_PATH, strerror(errno));
            exit_code = EXIT_FAILURE;
            break;
        }
    }

    /* Cleanup */

    if (ioctl(uinput_fd, UI_DEV_DESTROY) < 0) {
        fprintf(stderr, "Unable to destroy device \"%s\": %s\n",
                UINPUT_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Releasing exclusive access\n");
    if (ioctl(input_fd, EVIOCGRAB, 0) != 0) {
        fprintf(stderr, "Unable to release exclusive access to \"%s\": %s\n",
                device_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(uinput_fd) == -1) {
        fprintf(stderr, "Unable to close device \"%s\": %s\n",
                UINPUT_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(input_fd) == -1) {
        fprintf(stderr, "Unable to close device \"%s\": %s\n",
                device_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return exit_code;
}

void usage(char *name)
{
    printf("Usage: %s [KEY_CODE] [INPUT_PATH]\n", name);
}

void setup_uinput(int uinput_fd, int out_key)
{
    /* uinput device structure */
    struct uinput_user_dev uidev;

    if (ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY) < 0) {
        fprintf(stderr, "Unable to inform the input subsystem which types of "
                "input event we want to use: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ioctl(uinput_fd, UI_SET_KEYBIT, out_key) < 0) {
        fprintf(stderr, "Unable to inform the input subsystem which key we "
                "want to output: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "interceptkb");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if (write(uinput_fd, &uidev, sizeof(uidev)) < 0) {
        exit(EXIT_FAILURE);
    }

    if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
        exit(EXIT_FAILURE);
    }
}
