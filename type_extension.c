#include <Python.h>
#include <structmember.h>
#include "src/buffer.h"

static PyObject *InsufficientDataError;
static PyObject *FullError;

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    struct ring_buffer *buffer;
    int order;
    int closed;
} Buffer;

static void
Buffer_dealloc(Buffer* self)
{
    ring_buffer_free (self->buffer);
    self->ob_type->tp_free ((PyObject*) self);
}

static PyObject *
Buffer_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    Buffer* self;
    self = (Buffer*)type->tp_alloc(type, 0);

    return (PyObject *)self;
}

static int
Buffer_init(Buffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"order", NULL};

    if ( !PyArg_ParseTupleAndKeywords(args, kwargs, "|i", kwlist, &self->order) ) {
        return -1;
    }

    if (self->order == 0) {
        self->order = 12; // the ring buffer size defaults to 4KB
    }

    if (self->order < 12) {
        PyErr_SetString (PyExc_ValueError, "Order is too small, which has to be at least 12");
        return NULL;
    }

    self->buffer = malloc(sizeof *self->buffer);
    ring_buffer_create (self->buffer, self->order);

    return 0;
}

static PyObject *
Buffer_write(Buffer *self, PyObject *args, PyObject *kwargs)
{
    char *data;
    int count_bytes;
    static char *kwlist[] = {"data", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#", kwlist, &data, &count_bytes))
        return NULL;

    if (self->closed) {
        PyErr_SetString (PyExc_ValueError, "I/O operation on closed file");
        return NULL;
    }
    int available_bytes = ring_buffer_count_free_bytes (self->buffer);
    if (available_bytes < count_bytes) {
        PyErr_SetString(FullError, "Not enough free bytes to write");
        return NULL;
    }

    ring_buffer_write (self->buffer, data, count_bytes);

    return Py_None;
}

static PyObject * 
Buffer_close(Buffer *self, PyObject *args, PyObject *kwargs)
{
    ring_buffer_write_close (self->buffer);
    self->closed = 1;

    return Py_None;
}

static PyObject *
Buffer_eof(Buffer *self, PyObject *args, PyObject *kwargs)
{
    if ( self->closed && ring_buffer_eof(self->buffer) )
        return Py_True;
    return Py_False;
}

static PyObject *
Buffer_read(Buffer *self, PyObject *args, PyObject *kwargs)
{
    int count_bytes;
    char *kwlist[] = {"length", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &count_bytes))
        return NULL;

    int bytes_available_for_read = ring_buffer_count_bytes (self->buffer);
    if (bytes_available_for_read < count_bytes) {
        PyErr_SetString(InsufficientDataError, "Not enough data to read from");
        return NULL;
    }

    PyObject *datagram = PyString_FromStringAndSize(NULL, count_bytes);
    ring_buffer_read (self->buffer, PyString_AsString(datagram), count_bytes);
    return datagram;
}

static PyObject *
Buffer_peek_read(Buffer *self, PyObject *args, PyObject *kwargs)
{
    int count_bytes;
    char *kwlist[] = {"length", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &count_bytes))
        return NULL;

    int bytes_available_for_read = ring_buffer_count_bytes (self->buffer);
    if (bytes_available_for_read < count_bytes) {
        PyErr_SetString (InsufficientDataError, "Not enough data to read from");
        return NULL;
    }

    PyObject *datagram = PyString_FromStringAndSize(NULL, count_bytes);
    ring_buffer_peek (self->buffer, PyString_AsString(datagram), count_bytes);

    return datagram;
}

static PyObject *
Buffer_read_piece(Buffer *self, PyObject *args, PyObject *kwargs)
{
    if ( ring_buffer_eof(self->buffer) ) {
        PyErr_SetString (InsufficientDataError, "No data in buffer");
        return NULL;
    }

    int bytes_available_for_read = ring_buffer_count_bytes (self->buffer);
    PyObject *datagram;
    if (bytes_available_for_read < self->buffer->page_size) {
        datagram = PyString_FromStringAndSize(NULL, bytes_available_for_read);
        ring_buffer_read (self->buffer, PyString_AsString(datagram), bytes_available_for_read);
    } else {
        datagram = PyString_FromStringAndSize(NULL, self->buffer->page_size);
        ring_buffer_read (self->buffer, PyString_AsString(datagram), bytes_available_for_read);
    }

    return datagram;
}

static Py_ssize_t
Buffer_len(Buffer* self)
{
    return ring_buffer_count_bytes (self->buffer);
}

static PySequenceMethods Buffer_sequence_methods = {
    Buffer_len     /* sq_length */
};

static PyMemberDef Buffer_members[] = {
    {"order", T_INT, offsetof(Buffer, order), 0,
     "size of ring buffer represented by log2"},
    {NULL} /* Sentinel */
};

static PyMethodDef Buffer_methods[] = {
    {"write", (PyCFunction)Buffer_write, METH_VARARGS | METH_KEYWORDS,
     "Write a bytearray to the ring buffer"},
    {"read", (PyCFunction)Buffer_read, METH_KEYWORDS,
     "Read a certain amount of bytes from the buffer"},
    {"close", (PyCFunction)Buffer_close, METH_NOARGS,
     "Signal that the writing is done"},
    {"eof", (PyCFunction)Buffer_eof, METH_NOARGS,
     "Signal that end-of-file is reached"},
    {"peek_read", (PyCFunction)Buffer_peek_read, METH_KEYWORDS,
     "Read data without advancing the read pointer"},
    {"read_piece", (PyCFunction)Buffer_read_piece, METH_NOARGS,
     "Read a piece of buffer data efficiently"},
    {NULL} /* Sentinel */
};

static PyTypeObject buffer_BufferType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size: always set this to zero; retained for historical reason*/
    "ring_buffer.Buffer",           /*tp_name: textual representation and show up in error messages*/
    sizeof(Buffer),            /*tp_basicsize: how much to allocate when PyObject_New() is called*/
    0,                         /*tp_itemsize*/
    (destructor)Buffer_dealloc,/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    &Buffer_sequence_methods,    /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "ring_buffer.Buffer objects",          /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Buffer_methods,            /* tp_methods */
    Buffer_members,            /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Buffer_init,     /* tp_init */
    0,                         /* tp_alloc */
    Buffer_new,                /* tp_new */
};


static PyMethodDef module_methods[] = {
    {NULL}
};


#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
# define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initring_buffer(void)
{
    PyObject* m;
    PyObject* mod_bytearray_fifo;


    buffer_BufferType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&buffer_BufferType) < 0)
        return;

    m = Py_InitModule3("ring_buffer", module_methods,
                       "a ring buffer extension type.");
    if (m == NULL)
        return;

    InsufficientDataError = PyErr_NewExceptionWithDoc(
            "ring_buffer.InsufficientDataError",
            "Not enough data to read from",
            NULL,
            NULL);
    Py_INCREF(InsufficientDataError);
    PyModule_AddObject(m, "InsufficientDataError", InsufficientDataError);

    FullError = PyErr_NewExceptionWithDoc(
            "ring_buffer.FullError",
            "Not enough free space to write to",
            NULL,
            NULL);

    Py_INCREF(&buffer_BufferType);
    PyModule_AddObject(m, "Buffer", (PyObject *)&buffer_BufferType);
}
