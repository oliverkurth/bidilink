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
