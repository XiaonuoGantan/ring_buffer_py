#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"
#include "../src/buffer.h"

int tests_run = 0;

static struct ring_buffer *
construct_buffer()
{
    struct ring_buffer *buffer;
    buffer = malloc(sizeof *buffer);

    return buffer;
}

static char *
test_init()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12); // create a ring_buffer with a size of 4KB
    ring_buffer_free (buffer);

    mu_assert("ring_buffer created and freed",
              1 == 1);
    return 0;
}

static char *
test_address_ret()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    void *write_address = ring_buffer_write_address (buffer);
    mu_assert("ring_buffer write_address is correctly returned",
              buffer->address + buffer->write_offset_bytes == write_address);
    void *read_address = ring_buffer_read_address (buffer);
    mu_assert("ring_buffer read_address is correctly returned",
              buffer->address + buffer->read_offset_bytes == read_address);

    ring_buffer_free (buffer);
}

static char *
test_address_advance()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    void *write_address = ring_buffer_write_address (buffer);
    ring_buffer_write_advance (buffer, 1024);
    void *write_advanced_address = ring_buffer_write_address (buffer);
    mu_assert("ring_buffer write_address correctly advanced 1024 bytes",
              write_address + 1024 == write_advanced_address);

    void *read_address = ring_buffer_read_address (buffer);
    ring_buffer_read_advance (buffer, 1024);
    void *read_advanced_address = ring_buffer_read_address (buffer);
    mu_assert("ring_buffer read_address correctly advanced 1024 bytes",
              read_address + 1024 == read_advanced_address);

    ring_buffer_free (buffer);
}

static char *
test_write()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    void *start_write_address = ring_buffer_write_address (buffer);
    char data[] = "test";
    ring_buffer_write (buffer, data, 4UL);
    void *end_write_address = ring_buffer_write_address (buffer);
    mu_assert("ring_buffer write_address correctly advanced 4 bytes",
              start_write_address + 4 == end_write_address);

    ring_buffer_free (buffer);
}

static char *
test_read()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    char data[] = "test";
    ring_buffer_write (buffer, data, 4UL);
    char *read_data = malloc(sizeof(char) * 4);
    ring_buffer_read (buffer, read_data, 4UL);

    int i;
    int is_the_same = 0; // @data and @read_data should contain "test"
    for (i=0; i<4; i++) {
        if (data[i] != read_data[i]) {
            is_the_same = 1;
            break;
        }
    }

    mu_assert("ring_buffer read correctly returned the char[] array",
              is_the_same == 0);

    ring_buffer_free (buffer);
}

static char *
test_write_close()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    char data[] = "test";
    ring_buffer_write (buffer, data, 4UL);
    ring_buffer_write_close (buffer);

    mu_assert("ring_buffer_write_close sets the buffer->end_offset_bytes to buffer->write_offset_bytes",
              buffer->end_offset_bytes == buffer->write_offset_bytes);

    ring_buffer_free (buffer);
}

static char *
test_eof()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    char data[] = "test";
    ring_buffer_write (buffer, data, 4UL);
    ring_buffer_write_close (buffer);
    char *read_data = malloc(sizeof(char)*4);
    ring_buffer_read (buffer, data, 4UL);
    mu_assert("ring_buffer_eof should return a non-zero integer",
              ring_buffer_eof (buffer));

    ring_buffer_free (buffer);
}

static char *
test_peek()
{
    struct ring_buffer *buffer = construct_buffer();

    ring_buffer_create (buffer, 12);

    char data[] = "test";
    ring_buffer_write (buffer, data, 4UL);
    ring_buffer_write_close (buffer);

    char *peek_data = malloc(sizeof(char)*4);
    ring_buffer_peek (buffer, peek_data, 4UL);
    int i;
    int is_the_same = 1;
    for (i=0; i<4; i++) {
        if (data[i] != peek_data[i]) {
            is_the_same = 0;
            break;
        }
    }

    mu_assert("ring_buffer_peek should assign 4 chars to the char array peek_data",
              is_the_same);

    ring_buffer_free (buffer);
}

static char *all_tests()
{
    mu_run_test(test_init);
    mu_run_test(test_address_ret);
    mu_run_test(test_address_advance);
    mu_run_test(test_write);
    mu_run_test(test_read);
    mu_run_test(test_write_close);
    mu_run_test(test_eof);
    mu_run_test(test_peek);
    return 0;
}

int main(int argc, char **argv)
{
        char *result = all_tests();
        if (result != 0)
        {
            printf("%s\n", result);
        }
        else
        {
            printf("ALL TESTS PASSED\n");
        }
        printf("Tests run: %d\n", tests_run);

        return result != 0;
}
