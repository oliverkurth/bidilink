#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "server-tty.h"

static void destruct(struct stream *s) {
    assert(s && s->user);
    if (unlink((char*) s->user) < 0)
        fprintf(stderr, "WARNING: unlink(): %s\n", strerror(errno));
    free(s->user);
}

struct stream* stream_server_tty(const char *args) {
    struct stream *s = NULL;
    int fd = -1;
    char *n;
    
    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));

    if ((fd = open("/dev/ptmx", O_RDWR|O_NOCTTY)) < 0) {
        fprintf(stderr, "open('/dev/ptmx', O_RDWR|O_NOCTTY): %s\n", strerror(errno));
        goto fail;
    }

    if (grantpt(fd) < 0) {
        fprintf(stderr, "grantpt(): %s\n", strerror(errno));
        goto fail;
    }

    if (unlockpt(fd) < 0) {
        fprintf(stderr, "grantpt(): %s\n", strerror(errno));
        goto fail;
    }

    if (!(n = ptsname(fd)) < 0) {
        fprintf(stderr, "ptsname(): %s\n", strerror(errno));
        goto fail;
    }

    if (args) {
        if (symlink(n, args) < 0) {
            fprintf(stderr, "symlink('%s', '%s'): %s\n", n, args, strerror(errno));
            goto fail;
        }
    } else
        fprintf(stderr, "Allocated pseudo tty '%s'.\n", n);
    
    s->user = strdup(args);
    s->destruct = destruct;
    s->input_fd = s->output_fd = fd;
    return s;        

fail:
    free(s);
    if (fd >= 0)
        close(fd);

    return NULL;
    
}
