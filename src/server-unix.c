#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/un.h>
#include <unistd.h>

#include "server-unix.h"

struct stream* stream_server_unix(const char *args) {
    struct stream *s = NULL;
    int fd = -1, cfd;
    struct sockaddr_un sa;
    
    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        goto fail;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strncpy (sa.sun_path, args, sizeof(sa.sun_path)-1);

    if (bind(fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0) {
        fprintf(stderr, "bind(): %s\n", strerror(errno));
        goto fail;
    }

    if (listen(fd, 1) < 0) {
        fprintf(stderr, "listen(): %s\n", strerror(errno));
        goto fail;
    }

    if ((cfd = accept(fd, NULL, 0)) < 0) {
        fprintf(stderr, "accept(): %s\n", strerror(errno));
        goto fail;
    }

    close(fd);

    if (unlink(args) < 0)
        fprintf(stderr, "WARNING: unlink('%s'): %s\n", args, strerror(errno));
    
    s->input_fd = s->output_fd = cfd;

    return s;
    
fail:
    free(s);
    
    if (fd >= 0)
        close(fd);

    unlink(args);

    return NULL;
}
