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

#include "client-unix.h"

struct stream* stream_client_unix(const char *args) {
    struct stream *s = NULL;
    int fd = -1;
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

    if (connect(fd, (struct sockaddr *) &sa, SUN_LEN(&sa)) < 0) {
        fprintf(stderr, "connect(): %s\n", strerror(errno));
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
