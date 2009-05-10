/* -*- linux-c -*- */
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

#include "client-tcp6.h"

struct stream* stream_client_tcp6(const char *args) {
	struct stream *s = NULL;
	int fd = -1;
	struct addrinfo hints, *res, *ressave;
	size_t l;
	const char *port = "23";
	char hn[256];
	int n, save_errno = 0;

	l = strcspn(args, "/");
	if (l > sizeof(hn)-1)
		l = sizeof(hn)-1;
	strncpy(hn, args, l);
	hn[l] = 0;

	if (args[l] == '/')
		port = args+l+1;

	s = malloc(sizeof(struct stream));
	assert(s);
	memset(s, 0, sizeof(struct stream));\

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	n = getaddrinfo(hn, port, &hints, &res);

	if (n < 0) {
		fprintf(stderr,
			"getaddrinfo error:: [%s]\n",
			strerror(errno));
		goto fail;
	}
	
	ressave = res;

	fd =- 1;
	while (res) {
		fd = socket(res->ai_family,
				res->ai_socktype,
				res->ai_protocol);

		if (!(fd < 0)) {
			if(connect(fd, res->ai_addr, res->ai_addrlen) == 0)
				break;
			save_errno = errno;
			close(fd);
			fd = -1;
		}
		res = res->ai_next;
	}

	freeaddrinfo(ressave);
	if(fd < 0){
		fprintf(stderr, "connect(): %s\n", strerror(save_errno));
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
