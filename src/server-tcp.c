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

#include "server-tcp.h"

struct stream* stream_server_tcp(const char *args) {
    struct stream *s = NULL;
    int fd = -1, cfd;
    struct sockaddr_in sa;
    struct hostent *h;
    size_t l;
    uint16_t port = 23;
    char hn[256] = "";
    
    l = strcspn(args, ":");
    if (l > sizeof(hn)-1)
        l = sizeof(hn)-1;

    if (args[l] == ':') {
        strncpy(hn, args, l);
        hn[l] = 0;

        port = atoi(args+l+1);
    } else
        port = atoi(args);

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
    if (hn[0]) {
        if (!(h = gethostbyname(hn))) {
            fprintf(stderr, "Unknown host '%s'.\n", hn);
            goto fail;
        }
        sa.sin_addr = *(struct in_addr *) h->h_addr;
    } else
        sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        fprintf(stderr, "bind(): %s\n", strerror(errno));
        goto fail;
    }

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
