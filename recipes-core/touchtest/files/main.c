#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

typedef struct point {
    int x;
    int y;
} Point;

int touchloop(int fd, int xmin, int xmax, int ymin, int ymax, Point *p) {
    int resolution_xmax = 320;
    int resolution_ymax = 480;

    struct input_event ev = {0};
    do {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n == (ssize_t)-1) {
            if (errno == EINTR) continue;
            return -1;
        } else if (n != sizeof(ev)) {
            return -2;
        }

        switch (ev.type) {
        case EV_ABS:
            switch (ev.code) {
            case ABS_X:
                if (xmax > 0) {
                    double rate = 1 - (double)(ev.value - xmin) / (xmax - xmin);
                    p->x = (int)(rate * resolution_xmax);
                }
                break;
            case ABS_Y:
                if (ymax > 0) {
                    p->y = (int)((double)(ev.value - ymin) / (ymax - ymin) * resolution_ymax);
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    } while (ev.type != EV_SYN || ev.code != SYN_REPORT);
    return 0;
}

int main(void) {
    const char *device = "/dev/input/event0";
    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct input_absinfo absinfo;
    int xmin = 200, ymin = 200;
    int xmax = 3900, ymax = 3900;

    while(1) {
        Point p = {-1, -1};
        touchloop(fd, xmin, xmax, ymin, ymax, &p);
        if (p.x >= 0 && p.y >= 0) {
            printf("Touched at (%d, %d)\n", p.x, p.y);
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
