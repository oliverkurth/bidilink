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

#ifdef HAVE_POSIX_OPENPT
    fd = posix_openpt(O_RDWR|O_NOCTTY);
#else
    fd = open("/dev/ptmx", O_RDWR|O_NOCTTY);
#endif
    if (fd < 0) {
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
