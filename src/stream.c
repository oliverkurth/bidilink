#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include "stream.h"
#include "std.h"
#include "exec.h"
#include "client-tty.h"
#include "server-tty.h"
#include "client-tcp.h"
#include "server-tcp.h"
#include "client-unix.h"
#include "server-unix.h"

struct stream* stream_open(const char *spec) {
    if (!spec || !strncmp(spec, "std:",4))
        return stream_std(spec+4);
    else if (!strncmp(spec, "exec:", 5))
        return stream_exec(spec+5);
    else if (!strncmp(spec, "tty:", 4))
        return stream_client_tty(spec+4);
    else if (!strncmp(spec, "pty:", 4))
        return stream_server_tty(spec+4);
    else if (!strncmp(spec, "tcp-client:", 11))
        return stream_client_tcp(spec+11);
    else if (!strncmp(spec, "tcp-server:", 11))
        return stream_server_tcp(spec+11);
    else if (!strncmp(spec, "unix-client:", 12))
        return stream_client_unix(spec+12);
    else if (!strncmp(spec, "unix-server:", 12))
        return stream_server_unix(spec+12);

    fprintf(stderr, "Found no stream implementation for '%s'.\n", spec);
    return NULL;
}


void stream_close(struct stream *s) {
    assert(s);

    if (s->destruct)
        s->destruct(s);
    
    if (s->input_fd != -1)
        close(s->input_fd);
    if (s->output_fd != -1 && s->input_fd != s->output_fd)
        close(s->output_fd);

    free(s);
}
