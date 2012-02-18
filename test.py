import ring_buffer
import unittest


class BufferTestCase(unittest.TestCase):
    def setUp(self):
        self.buffer = ring_buffer.Buffer() # 4KB ring buffer

    def tearDown(self):
        del self.buffer

    def testReadWrite(self):
        data = b'1234'
        self.buffer.write(data)
        r_data = self.buffer.read(length=4)
        self.assertEquals(data, r_data)

    def testReadWriteCloseException(self):
        data = b'1234'
        self.buffer.write(data)
        r_data = self.buffer.read(length=4)
        self.buffer.close()
        self.assertRaises(ValueError, self.buffer.write, data)

    def testReadInsufficientDataException(self):
        data = b'1234'
        self.buffer.write(data)
        with self.assertRaises(ring_buffer.InsufficientDataError):
            self.buffer.read(length=5)

    def testEOF(self):
        data = b'1234'
        self.buffer.write(data)
        self.buffer.read(length=4)
        self.buffer.close()
        self.assertTrue(self.buffer.eof())

    def testPeekRead(self):
        data = b'1234'
        self.buffer.write(data)
        pr_data = self.buffer.read(length=4)
        self.assertEquals(pr_data, data)
        with self.assertRaises(ring_buffer.InsufficientDataError):
            self.buffer.read(length=4)

    def testReadPiece(self):
        data = b'1234'
        self.buffer.write(data)
        r_data = self.buffer.read_piece()
        self.assertEquals(r_data, data)
        with self.assertRaises(ring_buffer.InsufficientDataError):
            self.buffer.read_piece()

    def testMultipleReadWrite(self):
        data_1 = b'1234'
        data_2 = b'5678'
        self.buffer.write(data_1)
        self.buffer.write(data_2)
        r_data_1 = self.buffer.read(length=3)
        self.assertEquals(b'123', r_data_1)
        r_data_2 = self.buffer.read(length=5)
        self.assertEquals(b'45678', r_data_2)

    def testMixedReadWrite(self):
        data_1 = b'1234'
        data_2 = b'567'
        data_3 = b'890'
        self.buffer.write(data_1)
        r_data_1 = self.buffer.read(length=3)
        self.assertEquals(b'123', r_data_1)
        self.buffer.write(data_2)
        r_data_2 = self.buffer.read(length=2)
        self.assertEquals(b'45', r_data_2)
        self.buffer.write(data_3)
        r_data_3 = self.buffer.peek_read(length=3)
        self.assertEquals(b'678', r_data_3)
        r_data_4 = self.buffer.read_piece()
        self.assertEquals(b'67890', r_data_4)

    def testLen(self):
        self.assertEquals(0, len(self.buffer))
        data_1 = b'1234'
        self.buffer.write(data_1)
        self.assertEquals(4, len(self.buffer))
        self.buffer.read(4)
        self.assertEquals(0, len(self.buffer))
