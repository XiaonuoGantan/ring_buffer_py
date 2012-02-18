#ifndef _BSD_SOURCE
# define _BSD_SOURCE 1
#endif

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "buffer.h"


/* Construct a ring_buffer by passing a reference to the zero-filled initialized *buffer pointer
 * @buffer: the zero-filled ring_buffer pointer
 * @order: size of the buffer in log2, which has to be at least 12 on linux
 */
void
ring_buffer_create(struct ring_buffer *buffer, unsigned long order)
{
    int fd;
    void *address;
    int status;

    fd = open("/dev/zero", O_RDWR);
    if (fd < 0)
        terminate_and_generate_core_dump();

    buffer->count_bytes = (1UL << order);
    buffer->write_offset_bytes = 0;
    buffer->read_offset_bytes = 0;
    buffer->end_offset_bytes = 0;
    buffer->page_size = sysconf(_SC_PAGESIZE);

    //status = ftruncate(fd, buffer->count_bytes); // Truncate the fd into buffer->count_bytes
    //if (status)
    //    terminate_and_generate_core_dump();

    buffer->address = mmap (NULL, buffer->count_bytes << 1, PROT_NONE,
                            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (buffer->address == MAP_FAILED)
        terminate_and_generate_core_dump();

    /* Notice how this mmap call and the next mmap call map two physical memory pieces 
     * to the same file descriptor @fd with the same offset 0. This is why when the
     * write pointer advances beyond the buffer->count_bytes, the next write call will
     * start automatically from the beginning of the @fd.
     */
    address = mmap (buffer->address, buffer->count_bytes, PROT_READ | PROT_WRITE,
                    MAP_FIXED | MAP_SHARED, fd, 0);

    if (address != buffer->address)
        terminate_and_generate_core_dump();

    address = mmap (buffer->address + buffer->count_bytes,
                    buffer->count_bytes, PROT_READ | PROT_WRITE,
                    MAP_FIXED | MAP_SHARED, fd, 0);

    if (address != buffer->address + buffer->count_bytes)
        terminate_and_generate_core_dump();

    status = close (fd); // already has a mmap-ed ring_buffer pointer *address, do not need fd anymore
    if (status)
        terminate_and_generate_core_dump();
}

void
ring_buffer_free (struct ring_buffer *buffer)
{
    int status;

    if (buffer == NULL || buffer->address == NULL)
        return;

    status = munmap (buffer->address, buffer->count_bytes << 1);
    if (status)
        terminate_and_generate_core_dump();
}

void *
ring_buffer_write_address (struct ring_buffer *buffer)
{
    /* ignore warnings about void pointer arithmetic */
    return buffer->address + buffer->write_offset_bytes;
}

void
ring_buffer_write_advance (struct ring_buffer *buffer, unsigned long count_bytes)
{
    buffer->write_offset_bytes += count_bytes;
}

void *
ring_buffer_read_address (struct ring_buffer *buffer)
{
    return buffer->address + buffer->read_offset_bytes;
}

void
ring_buffer_read_advance (struct ring_buffer *buffer, unsigned long count_bytes)
{
    buffer->read_offset_bytes += count_bytes;

    if (buffer->read_offset_bytes >= buffer->count_bytes)
    {
        buffer->read_offset_bytes -= buffer->count_bytes;
        buffer->write_offset_bytes -= buffer->count_bytes;
    }
}

/* Return how many bytes available for read
 */
unsigned long
ring_buffer_count_bytes (struct ring_buffer *buffer)
{
    return buffer->write_offset_bytes - buffer->read_offset_bytes;
}

/* Return how many bytes available for write
 */
unsigned long
ring_buffer_count_free_bytes (struct ring_buffer *buffer)
{
    return buffer->count_bytes - ring_buffer_count_bytes (buffer);
}

void
ring_buffer_clear (struct ring_buffer *buffer)
{
    buffer->write_offset_bytes = 0;
    buffer->read_offset_bytes = 0;
}

/* Write @data of size @count_bytes into the buffer if there is enough space. Otherwise,
 * terminate_and_generate_core_dump()
 */
void
ring_buffer_write (struct ring_buffer *buffer, char *data, size_t count_bytes)
{
    // TODO: trigger an python exception instead of core dump
    if (ring_buffer_count_free_bytes (buffer) < count_bytes)
        terminate_and_generate_core_dump();
    memcpy ((void *)ring_buffer_write_address (buffer), (void *)data, count_bytes);
    ring_buffer_write_advance (buffer, count_bytes);
}

/* Read @count_bytes into the @data.
 */
void
ring_buffer_read (struct ring_buffer *buffer, char *data, unsigned long count_bytes)
{
    memcpy (data, ring_buffer_read_address (buffer), count_bytes);
    ring_buffer_read_advance (buffer, count_bytes);
}

/* Assign the value of current write_offset_bytes to end_offset_bytes to mark an eof
 */
void
ring_buffer_write_close (struct ring_buffer *buffer)
{
    buffer->end_offset_bytes = buffer->write_offset_bytes;
}

int ring_buffer_eof (struct ring_buffer *buffer)
{
    return buffer->write_offset_bytes == buffer->read_offset_bytes;
}

/* Copy the buffer data into @data without advancing the read pointer
 */
void
ring_buffer_peek (struct ring_buffer *buffer, char *data, int count_bytes)
{
    memcpy (data, ring_buffer_read_address (buffer), count_bytes);
}
