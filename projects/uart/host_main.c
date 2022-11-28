#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static speed_t get_speed(unsigned long baudrate)
{
    switch(baudrate) {
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 134:
            return B134;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        default:
            return B0;
    }
}

static void __attribute__((noreturn)) die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputs("\r\n", stderr);
    va_end(ap);
    abort();
}

static void lprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputs("\r\n", stderr);
    va_end(ap);
}

int main(int argc, char **argv)
{
    int c, tty_fd;
    char *scratch;
    char tty_devname[128] = { 0 };
    char tty_buffer[128] = { 0 };
    unsigned long tty_baudrate;
    speed_t tty_speed;
    struct termios tty_info = { 0 };
    int tty_parity = 0;
    int tty_flowcontrol = 0;
    int tty_doublestop = 0;
    unsigned long timeout = 5000;
    clock_t timeout_clock = 0;
    ssize_t readret = 0;

    /* Defaults */
    strncpy(tty_devname, "/dev/ttyUSB0", sizeof(tty_devname));
    tty_baudrate = 9600;
    tty_speed = B9600;

    while((c = getopt(argc, argv, "d:r:p:xst:h")) != -1) {
        switch(c) {
            case 'd':
                strncpy(tty_devname, optarg, sizeof(tty_devname));
                break;
            case 'r':
                tty_baudrate = strtoul(optarg, NULL, 10);
                tty_speed = get_speed(tty_baudrate);
                break;
            case 'p':
                if(!strcmp(optarg, "none"))
                    tty_parity = 0;
                else if(!strcmp(optarg, "odd"))
                    tty_parity = 1;
                else if(!strcmp(optarg, "even"))
                    tty_parity = 2;
                else
                    die("unknown argument: %s", optarg);
                break;
            case 'x':
                tty_flowcontrol = 1;
                break;
            case 's':
                tty_doublestop = 1;
                break;
            case 't':
                timeout = strtoul(optarg, NULL, 10);
                break;
            case 'h':
            case '?':
                lprintf("usage: %s [-d <device>] [-r <baudrate>] [-p <parity>] [-xs] [-h]", argv[0]);
                lprintf("options:");
                lprintf("   -d <device>     : specify the char device used as a terminal");
                lprintf("   -r <baudrate>   : specify the tty baud rate");
                lprintf("   -p <parity>     : set parity. values: none, odd, even");
                lprintf("   -x              : enable flow control (xon/xoff)");
                lprintf("   -s              : enable 1.5/2 stop bits");
                lprintf("   -h              : print this message and exit");
                return (c == 'h') ? 0 : 1;
        }
    }

    if(tty_speed == B0)
        die("invalid speed");

    tty_fd = open(tty_devname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(tty_fd == -1)
        die("%s: %s", tty_devname, strerror(errno));

    /* get terminal info */
    lprintf("%s: tcgeattr", tty_devname);
    tcgetattr(tty_fd, &tty_info);

    /* set baudrate */
    lprintf("%s: setting speed to %lu (%lu bytes/s)", tty_devname, tty_baudrate, tty_baudrate / 8);
    cfsetospeed(&tty_info, tty_speed);
    
    /* set default bits */
    tty_info.c_cflag &= ~CSTOPB;
    tty_info.c_cflag &= ~PARENB;
    tty_info.c_cflag &= ~PARODD;
    tty_info.c_iflag &= ~IXOFF;

    switch(tty_parity) {
        case 0:
            lprintf("%s: parity: disabled", tty_devname);
            break;
        case 1:
            lprintf("%s: parity: odd", tty_devname);
            tty_info.c_cflag |= PARENB;
            tty_info.c_cflag |= PARODD;
            break;
        case 2:
            lprintf("%s: parity: even", tty_devname);
            tty_info.c_cflag |= PARENB;
            break;
    }

    if(tty_flowcontrol) {
        lprintf("%s: flow control: enabled", tty_devname);
        tty_info.c_iflag |= IXOFF;
    }

    if(tty_doublestop) {
        lprintf("%s: 1.5/2 stop bits: enabled", tty_devname);
        tty_info.c_cflag |= CSTOPB;
    }

    lprintf("%s: tcsetattr", tty_devname);
    tcsetattr(tty_fd, TCSANOW, &tty_info);

    lprintf("%s: starting command exchange", tty_devname);

    while(!feof(stdin)) {
        fprintf(stdout, "@ ");

        if(!fgets(tty_buffer, sizeof(tty_buffer), stdin)) {
            lprintf("[EOF]");
            break;
        }

        if((scratch = strrchr(tty_buffer, '\n')) != NULL)
            scratch[0] = 0;
        write(tty_fd, tty_buffer, strlen(tty_buffer));

        timeout_clock = clock();

        for(;;) {
            readret = read(tty_fd, tty_buffer, sizeof(tty_buffer));
            
            if(readret > 0) {
                timeout_clock = 0;
                fprintf(stdout, "%s\n", tty_buffer);
                continue;
            }

            if(readret == 0)
                break;

            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                if((unsigned long)((float)(clock() - timeout_clock) / (float)(CLOCKS_PER_SEC) * 1000.0f) >= timeout) {
                    lprintf("%s: timed out: device didn't answer for %lu ms", tty_devname, timeout);
                    break;
                }

                continue;
            }

            lprintf("%s: read failed: %s", tty_devname, strerror(errno));
            break;
        }
    }

    close(tty_fd);

    return 0;
}
