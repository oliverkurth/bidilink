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
