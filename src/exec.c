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

#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "exec.h"

static void close_pipe(int p[2]) {
    if (p[0] >= 0)
        close(p[0]);

    if (p[1] >= 0)
        close(p[1]);
}

struct stream* stream_exec(const char *args) {
    struct stream *s = NULL;
    int stdout_pipe[2];
    int stdin_pipe[2];
    pid_t pid;

    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));    

    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
        fprintf(stderr, "pipe(): %s\n", strerror(errno));
        goto fail;
    }

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork(): %s\n", strerror(errno));
        goto fail;
    } else if (pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        if (dup2(stdin_pipe[0], 0) < 0 || dup2(stdout_pipe[1], 1) < 0) {
            fprintf(stderr, "dup2(): %s\n", strerror(errno));
            exit(1);
        }

        execl("/bin/sh", "/bin/sh", "-c", args, NULL);

        fprintf(stderr, "exec(): %s\n", strerror(errno));
        exit(1);
    }
    
    s->output_fd = stdin_pipe[1];
    close(stdin_pipe[0]);
    s->input_fd = stdout_pipe[0];
    close(stdout_pipe[1]);
    
    return s;
    
fail:

    free(s);

    close_pipe(stdout_pipe);
    close_pipe(stdin_pipe);
    
    return NULL;
}
