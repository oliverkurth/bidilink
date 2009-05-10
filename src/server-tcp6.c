/* -*- linux-c -*- */
/* $Id: server-tcp.c 4 2004-01-14 18:14:29Z lennart $ */
 
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

#include "server-tcp6.h"

struct stream* stream_server_tcp6(const char *args) {
	struct stream *s = NULL;
	int fd = -1, cfd;
	struct addrinfo hints, *res, *ressave;
	size_t l;
	const char *port = "23";
	char hn[256] = "";
	int n;

	l = strcspn(args, "/");
	if (l > sizeof(hn)-1)
		l = sizeof(hn)-1;

	if (args[l] == '/') {
		strncpy(hn, args, l);
		hn[l] = 0;

		port = args+l+1;
	}

	s = malloc(sizeof(struct stream));
	assert(s);
	memset(s, 0, sizeof(struct stream));

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	fprintf(stderr, "hn = %s, port = %s\n", hn, port);
	n = getaddrinfo(hn[0] ? hn : "::", port, &hints, &res);

	if (n < 0) {
		fprintf(stderr,
			"getaddrinfo error:: [%s]\n",
			strerror(errno));
		goto fail;
	}

	ressave = res;

	fd=-1;
	while (res) {
		fd = socket(res->ai_family,
			    res->ai_socktype,
			    res->ai_protocol);

		if (!(fd < 0)) {
			if (bind(fd, res->ai_addr, res->ai_addrlen) == 0)
				break;

			close(fd);
			fd = -1;
		}
		res = res->ai_next;
	}

	freeaddrinfo(ressave);
	if(fd < 0)
		goto fail;

	if (listen(fd, 1) < 0) {
		fprintf(stderr, "listen(): %s\n", strerror(errno));
		goto fail;
	}

	if ((cfd = accept(fd, NULL, 0)) < 0) {
		fprintf(stderr, "accept(): %s\n", strerror(errno));
		goto fail;
	}

	close(fd);
	s->input_fd = s->output_fd = cfd;

	return s;
    
fail:
	free(s);
	if (fd >= 0)
		close(fd);

	return NULL;
}
