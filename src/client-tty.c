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
#include <limits.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>

#include "client-tty.h"

static const char *lockfile(const char *dev) {
    static char lockfile[PATH_MAX];
    char *p = strrchr(dev, '/');
    if (p)
        p++;
    else
        p = (char*) dev;
    
    snprintf(lockfile, sizeof(lockfile), "%s/LCK..%s", LOCKDIR, p);
    return lockfile;
}

static const char *tempfile(const char *path) {
    static char t[PATH_MAX];
    snprintf(t, sizeof(t), "%s.tmp.%lu", path, (unsigned long) getpid());
    return t;
}

static int device_lock(const char *dev) {
    struct stat st;
    int fd;
    const char *path, *temp;
    char buf[100];
    char uidbuf[32];

    if (stat(LOCKDIR, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Failed to lock device, directory "LOCKDIR" not existent.\n");
        return -1;
    }

    path = lockfile(dev);
    temp = tempfile(path);

    for (;;) {
        mode_t u;
        struct passwd* pw;
        char *username;
        
        if ((fd = open(path, O_RDONLY)) < 0) {
            if (errno != ENOENT) {
                fprintf(stderr, "Failed to open lock file: %s\n", strerror(errno));
                return -1;
            }
        }
        
        if (fd >= 0) {
            ssize_t n;

            n = read(fd, buf, sizeof(buf) - 1);
            close(fd);

            if (n < 0) {
                close(fd);
                fprintf(stderr, "Failed to read from lock file: %s\n", strerror(errno));
                return -1;
            }
            
            if (n > 0) {
                pid_t pid;
                
                if (n == 4)
                    pid = (pid_t) *((uint32_t*) buf);
                else {
                    unsigned long v;
                    buf[n] = 0;
                    sscanf(buf, "%lu", &v);
                    pid = (pid_t) v;
                }
                
                if (pid > 0) {
                    if (kill(pid, 0) < 0 && errno == ESRCH) {
                        fprintf(stderr, "Lockfile is stale. Overriding it.\n");
                        /* Yes, here is a race condition */
                        unlink(path);
                    } else
                        return 1;
                }
            }
        }

        u = umask(0033);
        fd = open(temp, O_WRONLY | O_CREAT | O_EXCL, 0666);
        umask(u);
        
        if (fd < 0) {
            fprintf(stderr, "Failed to create temporary lock file: %s\n", strerror(errno));
            return -1;
        }

        if ((pw = getpwuid(getuid())))
            username = pw->pw_name;
        else
            snprintf(username = uidbuf, sizeof(uidbuf), "%lu", (unsigned long) getuid());

        snprintf(buf, sizeof(buf), "%10lu %s %.20s\n", (unsigned long) getpid(), "bidilink", username);
        if (write(fd, buf, strlen(buf)) < 0) {
            fprintf(stderr, "Failed to write to temporary lock file: %s\n", strerror(errno));
            close(fd);
            return -1;
        }

        close(fd);

        if (link(temp, path) < 0) {
            if (errno == EEXIST)
                continue;

            fprintf(stderr, "Failed to link temporary lock file: %s\n", strerror(errno));
        }

        unlink(temp);

        return 0;
    }
}

static int device_unlock(const char *dev) {
    if (unlink(lockfile(dev)) < 0) {
        fprintf(stderr, "Failed to remove lock file: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static void destruct(struct stream *s) {
    assert(s && s->user);

    device_unlock((char*) s->user);
    free(s->user);
}

struct stream* stream_client_tty(const char *args) {
    struct stream *s = NULL;
    struct termios ts;
    int fd = -1;
    int r;
    
    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));

    if ((r = device_lock(args)) != 0) {
        if (r > 0)
            fprintf(stderr, "Device locked.\n");
        goto fail;
    }

    
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
