#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jstring.h>

#include <uv.h>

#define BUFFER_LEN 1024

typedef struct read_write_op_s
{
    char*   buff;
    int     read_fd;
} read_write_op_t;

void on_read(uv_fs_t* req);
void on_write(uv_fs_t* req);

void on_open(uv_fs_t* req)
{
    uv_fs_t read_req;
    char* buff = NULL;
    read_write_op_t* read_write_op = malloc(sizeof(read_write_op_t));

    read_write_op->read_fd  = req->result;

    read_req.data = read_write_op;

    if (req->result != -1)
    {
        buff = malloc(BUFFER_LEN * sizeof(*buff));
        read_write_op->buff  = buff;
        uv_fs_read(uv_default_loop(),
                   &read_req,
                   req->result,
                   buff,
                   BUFFER_LEN,
                   -1,
                   on_read);
    }
    else
    {
        fprintf(stderr, "Error opening file: %d\n", req->errorno);
    }

    uv_fs_req_cleanup(req);
}

void on_read(uv_fs_t* req)
{
    ssize_t size_read = req->result;
    read_write_op_t* read_write_op = req->data;

    uv_fs_req_cleanup(req);

    if (size_read < 0)
    {
        fprintf(stderr, "Error when reading: %s\n", uv_strerror(uv_last_error(uv_default_loop())));
    }
    else if (size_read == 0)
    {
        uv_fs_t close_req;

        if (read_write_op && read_write_op->buff)
        {
            free(read_write_op->buff);
            read_write_op->buff = NULL;
        }

        uv_fs_close(uv_default_loop(), &close_req, read_write_op->read_fd, NULL);
    }
    else
    {
        uv_fs_t     write_req;
        write_req.data = read_write_op;

        uv_fs_write(uv_default_loop(),
                    &write_req,
                    1,
                    read_write_op->buff,
                    size_read,
                    -1,
                    on_write);
    }
}

void on_write(uv_fs_t* req)
{
    read_write_op_t* read_write_op = req->data;

    uv_fs_req_cleanup(req);

    if (req->result < 0)
    {
        fprintf(stderr, "Error when writing: %s\n", uv_strerror(uv_last_error(uv_default_loop())));
    }
    else
    {
        uv_fs_t read_req;
        read_req.data = read_write_op;

        uv_fs_read(uv_default_loop(),
                   &read_req,
                   read_write_op->read_fd,
                   read_write_op->buff,
                   BUFFER_LEN,
                   -1,
                   on_read);
    }
}

void on_message_written(uv_fs_t* req)
{
    uv_fs_t open_req;

    uv_fs_req_cleanup(req);

    uv_fs_open(uv_default_loop(), &open_req, req->data, O_RDONLY, 0, on_open);
}

int main(int argc, char* argv[])
{
    char* format_string = NULL;
    uv_fs_t write_req;

    format_string = jstr_format("Reading file [%s]...\n", argv[1]);
    write_req.data = argv[1];

    uv_fs_write(uv_default_loop(), &write_req, 1, format_string, strlen(format_string), -1, on_message_written);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    uv_fs_req_cleanup(&write_req);

    return 0;
}
