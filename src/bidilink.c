/* $Id$ */
 
/***
  This file is part of bidilink.
 
  bidilink is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
   
  bidilink is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.
   
  You should have received a copy of the GNU General Public License
  along with bidilink; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***/
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/select.h>
#include <signal.h>
#include <inttypes.h>

#include "stream.h"

#define BUFSIZE (64*1024)

volatile int quit = 0;

static void signal_handler(int s) {
    quit = 1;
}

static void usage(FILE *f, const char *argv0) {
    fprintf(f,
            "%s [-h|--help] [-v|--verbose] STREAM1 [STREAM2]\n\n"
            "Supported stream types:\n"
            "\tstd:\n"
            "\texec:PROGRAM\n"
            "\ttty:TTYDEVICE\n"
            "\tpty:PTYNAME\n"
            "\ttcp-client:HOSTNAME:PORT\n"
            "\ttcp-server:[IPADDRESS:]PORT\n"
            "\tunix-client:SOCKNAME\n"
            "\tunix-server:SOCKNAME\n",
            argv0);
}

static int read_fd(struct stream *s, void *buf, size_t *buf_fill, size_t *buf_index) {
    ssize_t r;
    assert(s && buf && buf_fill && buf_index);

    if ((r = read(s->input_fd, buf, BUFSIZE)) <= 0) {
        
        if (r == 0)
            return 0;
        
        fprintf(stderr, "Failure to read(): %s\n", strerror(errno));
        return -1;
    }
    
    *buf_fill = r;
    *buf_index = 0;
    return 1;
}


static int write_fd(struct stream *s, void *buf, size_t *buf_fill, size_t *buf_index) {
    ssize_t r;
    assert(s && buf && buf_fill && buf_index);
    
    if ((r = write(s->output_fd, buf+*buf_index, *buf_fill)) < 0) {
        
        if (r == EPIPE)
            return 0;
        
        fprintf(stderr, "Failure to write(): %s\n", strerror(errno));
        return -1;
    }
    
    *buf_fill -= r;
    *buf_index += r;
    return 1;
}

const char *byte_str(uint64_t l) {
    static char c[32];

    if (l <= 1024)
        snprintf(c, sizeof(c), "%llu B", l);
    else if (l <= 1024*1024)
        snprintf(c, sizeof(c), "%llu KB", l/1024);
    else if (l <= 1024*1024*1024)
        snprintf(c, sizeof(c), "%llu MB", l/1024/1024);
    else
        snprintf(c, sizeof(c), "%llu GB", l/1024/1024/1024);

    return c;
};

int main(int argc, char *argv[]) {
    struct stream *a = NULL, *b = NULL;

    void *ab_buf = NULL, *ba_buf = NULL;
    size_t ab_buf_fill = 0, ab_buf_index = 0,
        ba_buf_fill = 0, ba_buf_index = 0;

    uint64_t ab_count = 0, ba_count = 0;

    int a_readable = 0,
        a_writable = 0,
        b_readable = 0,
        b_writable = 0,
        verbose = 0;

    int ret = -1;
    int ai;
    
    if (argc < 2) {
        usage(stderr, argv[0]);
        ret = 0;
        goto finish;
    }

    signal(SIGINT, signal_handler);
    siginterrupt(SIGINT, 1);
    signal(SIGTERM, signal_handler);
    siginterrupt(SIGTERM, 1);
    signal(SIGPIPE, SIG_IGN);

    ai = 1;

    if (!strcmp(argv[ai], "-h") || !strcmp(argv[ai], "--help")) {
        usage(stdout, argv[0]);
        ret = 0;
        goto finish;
    }
    
    if (!strcmp(argv[ai], "-v") || !strcmp(argv[ai], "--verbose")) {
        verbose = 1;
        ai++;
    }

    if (!(a = stream_open(argv[ai++])))
        goto finish;

    if (!(b = stream_open(argc > ai ? argv[ai++] : NULL)))
        goto finish;

    ab_buf = malloc(BUFSIZE);
    assert(ab_buf);
    ba_buf = malloc(BUFSIZE);
    assert(ba_buf);

    for (;;) {
        fd_set ifds, ofds;

        
        FD_ZERO(&ifds);

        if (!a_readable)
            FD_SET(a->input_fd, &ifds);
        if (!b_readable)
            FD_SET(b->input_fd, &ifds);

        FD_ZERO(&ofds);
        
        if (!a_writable)
            FD_SET(a->output_fd, &ofds);
        if (!b_writable)
            FD_SET(b->output_fd, &ofds);

        if (select(FD_SETSIZE, &ifds, &ofds, NULL, 0) < 0) {
            if (errno == EINTR)
                continue;

            fprintf(stderr, "select() failed: %s\n", strerror(errno));
            ret = -1;
            break;
        }

        if (FD_ISSET(a->input_fd, &ifds))
            a_readable = 1;
        if (FD_ISSET(b->input_fd, &ifds))
            b_readable = 1;

        if (FD_ISSET(a->output_fd, &ofds))
            a_writable = 1;
        if (FD_ISSET(b->output_fd, &ofds))
            b_writable = 1;

        
        if (!ab_buf_fill && a_readable && b_writable) {
            int r;
            if ((r = read_fd(a, ab_buf, &ab_buf_fill, &ab_buf_index)) <= 0) {
                ret = r < 0 ? 1 : 0;
                break;
            }
            a_readable = 0;

            ab_count += ab_buf_fill;
        }

        if (ab_buf_fill && b_writable) {
            int r;
            if ((r = write_fd(b, ab_buf, &ab_buf_fill, &ab_buf_index)) <= 0) {
                ret = r < 0 ? 1 : 0;
                break;
            }
            b_writable = 0;
        }

        if (!ba_buf_fill && b_readable && a_writable) {
            int r;
            if ((r = read_fd(b, ba_buf, &ba_buf_fill, &ba_buf_index)) <= 0) {
                ret = r < 0 ? 1 : 0;
                break;
            }
            b_readable = 0;
            ba_count += ba_buf_fill;
        }

        if (ba_buf_fill && a_writable) {
            int r;
            if ((r = write_fd(a, ba_buf, &ba_buf_fill, &ba_buf_index)) <= 0) {
                ret = r < 0 ? 1 : 0;
                break;
            }
            a_writable = 0;
        }

        if (verbose) {
            fprintf(stderr, "A: %s; ", byte_str(ab_count));
            fprintf(stderr, "B: %s                      \r", byte_str(ba_count));
        }
    }

finish:

    free(ab_buf);
    free(ba_buf);

    if (a)
        stream_close(a);
    if (b)
        stream_close(b);

    return ret;
    
}
