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

#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <netdb.h>
#include <unistd.h>

#include "client-tcp.h"

struct stream* stream_client_tcp(const char *args) {
    struct stream *s = NULL;
    int fd = -1;
    struct sockaddr_in sa;
    struct hostent *h;
    size_t l;
    uint16_t port = 23;

    char hn[256];
    l = strcspn(args, ":");
    if (l > sizeof(hn)-1)
        l = sizeof(hn)-1;
    strncpy(hn, args, l);
    hn[l] = 0;

    if (args[l] == ':')
        port = atoi(args+l+1);

    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));

    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        goto fail;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (!(h = gethostbyname(hn))) {
        fprintf(stderr, "Unknown host '%s'.\n", hn);
        goto fail;
    }
    sa.sin_addr = *(struct in_addr *) h->h_addr;

    if (connect(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
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
