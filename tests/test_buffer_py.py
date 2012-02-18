from unittest import TestCase

import ring_buffer

from ring_buffer import (
    InsufficientDataError,
    FullError
)


class RingBufferModuleTest(TestCase):
    def test_has_exceptions(self):
        self.assertTrue(hasattr(ring_buffer, 'InsufficientDataError'))
        self.assertTrue(hasattr(ring_buffer, 'FullError'))

    def test_exceptions_are_libbrowzoo(self):
        self.assertTrue(
            ring_buffer.InsufficientDataError is InsufficientDataError)
        self.assertTrue(ring_buffer.FullError is FullError)


class BufferTest(TestCase):
    def test_full_condition(self):
        buf = ring_buffer.Buffer(order=12)
        buf.write("A"*1000)
        buf.write("B"*1000)
        buf.write("C"*1000)
        buf.write("D"*1000)
        with self.assertRaises(ring_buffer.FullError):
            buf.write("E"*1000)


    def test_empty_condition(self):
        buf = ring_buffer.Buffer()
        self.assertRaises(ring_buffer.InsufficientDataError, buf.read, 10)

    def test_constructor(self):
        # this one aborts
        buf = ring_buffer.Buffer(order=8)

