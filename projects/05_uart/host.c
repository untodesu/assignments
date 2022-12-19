#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputs("\r\n", stderr);
    va_end(ap);
}

static void noreturn die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputs("\r\n", stderr);
    va_end(ap);
    exit(1);
}

int main(int argc, char **argv)
{
    int fd, res;
    speed_t speed;
    struct termios tc;
    char c;

    if(argc < 2) {
        lprintf("usage: %s <ttydev>", argv[0]);
        die("not enough arguments");
    }

    /* force STDIN_FILENO to not block the input*/
    fcntl(STDIN_FILENO, F_SETFL, O_WRONLY | O_NONBLOCK);

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if(fd == -1)
        die("open %s: %s", argv[1], strerror(errno));

    if(!isatty(fd)) {
        close(fd);
        die("%s: not a tty", argv[1]);
    }

    if(tcgetattr(fd, &tc) == -1) {
        close(fd);
        die("(%s) tcgetaddr: %s", argv[1], strerror(errno));
    }
    
    tc.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | ISTRIP | IXON);
    tc.c_oflag &= ~(OCRNL | ONLCR | ONOCR | OFILL | OPOST);
    tc.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    /* 8-bits, even parity */
    tc.c_cflag &= ~(CSIZE | PARODD);
    tc.c_cflag |=  (PARENB | CS8);

    tc.c_cc[VMIN] = 1;
    tc.c_cc[VTIME] = 0;

    speed = B57600;
    if(cfsetispeed(&tc, speed) == -1 || cfsetospeed(&tc, speed) == -1) {
        close(fd);
        die("(%s) cfsetxspeed: %s", argv[1], strerror(errno));
    }

    if(tcsetattr(fd, TCSANOW, &tc) == -1) {
        close(fd);
        die("(%s) tcsetattr: %s", argv[1], strerror(errno));
    }

    for(;;) {
        res = read(STDIN_FILENO, &c, sizeof(c));
        if(res == 0) {
            lprintf("\n[EOF]");
            break;
        }
        else if(res > 0) {
            /* send to tty */
            write(fd, &c, sizeof(c));
        }

        if(read(fd, &c, sizeof(c)) && c) {
            /* send to stdout */
            write(STDERR_FILENO, &c, sizeof(c));

            if(res > 0) {
                /* do not throttle */
                continue;
            }
        }

        /* throttle */
        usleep(1000);
    }

    close(fd);

    return 0;
}
