#ifndef foostreamhfoo
#define foostreamhfoo


struct stream {
    int input_fd, output_fd;

    void (*destruct) (struct stream *s);

    void* user;
};

struct stream* stream_open(const char *sspec);
void stream_close(struct stream *s);


#endif
