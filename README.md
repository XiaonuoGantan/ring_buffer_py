# ring_buffer_py

The Python interface to the Optimized POSIX ring buffer implementation.

## Try it

In a virtual env, run:

    $ python setup.py develop

    $ sudo su

    $ nosetests test.py

Need to be root in order to execute the test program since the ring buffer C library uses mmap to allocate
virtual memory on /dev/shm.
