#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "std.h"

struct stream* stream_std(const char *args) {
    struct stream *s;

    s = malloc(sizeof(struct stream));
    assert(s);
    memset(s, 0, sizeof(struct stream));

    s->input_fd = 0;
    s->output_fd = 1;

    return s;
}
