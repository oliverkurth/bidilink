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
