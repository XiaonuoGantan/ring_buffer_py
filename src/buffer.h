#ifndef BUFFER_H
#define BUFFER_H

#define terminate_and_generate_core_dump() abort ()


struct ring_buffer
{
    void *address;

    unsigned long count_bytes; // buffer size in bytes
    unsigned long write_offset_bytes;
    unsigned long read_offset_bytes;
    unsigned long end_offset_bytes; // when an IWriteEndpoint calls close(), the end_offset_bytes is assigned
                                    // the value of write_offset_bytes
    long page_size; // unit of memory in bytes which is used by mmap to allocate memory
};

void ring_buffer_create (struct ring_buffer *buffer, unsigned long order);
void ring_buffer_free (struct ring_buffer *buffer);
void * ring_buffer_write_address (struct ring_buffer *buffer);
void ring_buffer_write_advance (struct ring_buffer *buffer, unsigned long count_bytes);
void * ring_buffer_read_address (struct ring_buffer *buffer);
void ring_buffer_read_advance (struct ring_buffer *buffer, unsigned long count_bytes);
unsigned long ring_buffer_count_bytes (struct ring_buffer *buffer);
unsigned long ring_buffer_count_free_bytes (struct ring_buffer *buffer);
void ring_buffer_clear (struct ring_buffer *buffer);

/* For libbrowzoo.python.interface.stream.IReadEndpoint and IWriteEndpoint */
void ring_buffer_write (struct ring_buffer *buffer, char *data, unsigned long count_bytes);
void ring_buffer_read (struct ring_buffer *buffer, char *data, unsigned long count_bytes);
void ring_buffer_write_close (struct ring_buffer *buffer);
int ring_buffer_eof (struct ring_buffer *buffer);
void ring_buffer_peek (struct ring_buffer *buffer, char *data, int count_bytes);

#endif
