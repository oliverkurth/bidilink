#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "client-tty.h"

struct stream* stream_client_tty(const char *args) {
    struct stream *s = NULL;
    struct termios ts;
    int fd = -1;
    
    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));
    
    if ((fd = open(args, O_RDWR|O_NOCTTY)) < 0) {
        fprintf(stderr, "open('%s', O_RDWR|O_NOCTTY): %s\n", args, strerror(errno));
        goto fail;
    }

    if (tcgetattr(fd, &ts) < 0) {
        fprintf(stderr, "tcgetattr(): %s\n", strerror(errno));
        goto fail;
    }

    cfmakeraw(&ts);
    
    ts.c_cflag |= HUPCL | CS8 | CLOCAL;
    ts.c_iflag |= IGNPAR | IGNBRK;

    ts.c_cc[VMIN] = 1;
    ts.c_cc[VTIME] = 0;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &ts) < 0) {
        fprintf(stderr, "tcsetattr(): %s\n", strerror(errno));
        goto fail;
    }

    s->input_fd = s->output_fd = fd;
    return s;        

fail:
    free(s);
    if (fd >= 0)
        close(fd);

    return NULL;
}
