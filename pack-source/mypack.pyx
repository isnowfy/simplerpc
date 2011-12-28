# coding: utf-8
#cython: embedsignature=True

from cpython cimport *
cdef extern from "Python.h":
    ctypedef char* const_char_ptr "const char*"
    ctypedef char* const_void_ptr "const void*"
    ctypedef struct PyObject
    cdef int PyObject_AsReadBuffer(object o, const_void_ptr* buff, Py_ssize_t* buf_len) except -1

from libc.stdlib cimport *
from libc.string cimport *
import gc
_gc_disable = gc.disable
_gc_enable = gc.enable

cdef extern from "pack.h":
    struct msgpack_packer:
        char* buf
        size_t length
        size_t buf_size

    int mypack(PyObject* o,int nest_limit,msgpack_packer* pk)


cdef int DEFAULT_RECURSE_LIMIT=511

cdef class Packer(object):
    """MessagePack Packer

    usage:

        packer = Packer()
        astream.write(packer.pack(a))
        astream.write(packer.pack(b))
    """
    cdef msgpack_packer pk

    def __cinit__(self):
        cdef int buf_size=1024*1024 
        self.pk.buf=<char*>malloc(buf_size)
        if self.pk.buf==NULL:
            raise MemoryError("unable to allocate internal buffer.")
        self.pk.buf_size=buf_size
        self.pk.length=0

    def __dealloc__(self):
        free(self.pk.buf)


    def pack(self, object obj):
        ret = mypack(<PyObject *>obj, DEFAULT_RECURSE_LIMIT, &self.pk)
        if ret:
            raise TypeError
        buf= PyBytes_FromStringAndSize(self.pk.buf, self.pk.length)
        self.pk.length=0
        return buf


def pack(object o, object stream):
    """
    pack an object `o` and write it to stream)."""
    packer = Packer()
    stream.write(packer.pack(o))

def packb(object o):
    """
    pack o and return packed bytes."""
    packer = Packer()
    return packer.pack(o)


cdef extern from "unpack.h":
    ctypedef struct template_context:
        unsigned int cs
        unsigned int trail
        unsigned int top
        PyObject* stack

    int template_execute(template_context* ctx, const_char_ptr data,
                         size_t len, size_t* off) except -1
    void template_init(template_context* ctx)
    object template_data(template_context* ctx)


def unpackb(object packed):
    """
    Unpack packed_bytes to object. Returns an unpacked object."""
    cdef template_context ctx
    cdef size_t off = 0
    cdef int ret

    cdef char* buf
    cdef Py_ssize_t buf_len
    PyObject_AsReadBuffer(packed, <const_void_ptr*>&buf, &buf_len)

    template_init(&ctx)
    _gc_disable()
    try:
        ret = template_execute(&ctx, buf, buf_len, &off)
    finally:
        _gc_enable()
    if ret == 1:
        return template_data(&ctx)
    else:
        return None


def unpack(object stream, object object_hook=None, object list_hook=None, bint use_list=0, encoding=None, unicode_errors="strict"):
    """
    unpack an object from stream.
    """
    return unpackb(stream.read(), use_list=use_list,
                   object_hook=object_hook, list_hook=list_hook, encoding=encoding, unicode_errors=unicode_errors)

cdef class Unpacker(object):
    """
    Streaming unpacker.
    read_size is used like file_like.read(read_size)

    `file_like` is a file-like object having `.read(n)` method.
    When `Unpacker` initialized with `file_like`, unpacker reads serialized data
    from it and `.feed()` method is not usable.

    `read_size` is used as `file_like.read(read_size)`. (default: 1M)

    example::

        unpacker = Unpacker()
        while 1:
            buf = astream.read()
            unpacker.feed(buf)
            for o in unpacker:
                do_something(o)
    """
    cdef template_context ctx
    cdef char* buf
    cdef size_t buf_size, buf_head, buf_tail
    cdef object file_like
    cdef object file_like_read
    cdef Py_ssize_t read_size

    def __cinit__(self):
        self.buf = NULL

    def __dealloc__(self):
        free(self.buf)
        self.buf = NULL

    def __init__(self, file_like=None, Py_ssize_t read_size=1024*1024):
        self.file_like = file_like
        if file_like:
            self.file_like_read = file_like.read
            if not PyCallable_Check(self.file_like_read):
                raise ValueError("`file_like.read` must be a callable.")
        self.read_size = read_size
        self.buf = <char*>malloc(read_size)
        if self.buf == NULL:
            raise MemoryError("Unable to allocate internal buffer.")
        self.buf_size = read_size
        self.buf_head = 0
        self.buf_tail = 0
        template_init(&self.ctx)

    def feed(self, object next_bytes):
        cdef char* buf
        cdef Py_ssize_t buf_len
        if self.file_like is not None:
            raise AssertionError(
                    "unpacker.feed() is not be able to use with`file_like`.")
        PyObject_AsReadBuffer(next_bytes, <const_void_ptr*>&buf, &buf_len)
        self.append_buffer(buf, buf_len)

    cdef append_buffer(self, void* _buf, Py_ssize_t _buf_len):
        cdef:
            char* buf = self.buf
            size_t head = self.buf_head
            size_t tail = self.buf_tail
            size_t buf_size = self.buf_size
            size_t new_size

        if tail + _buf_len > buf_size:
            if ((tail - head) + _buf_len)*2 < buf_size:
                # move to front.
                memmove(buf, buf + head, tail - head)
                tail -= head
                head = 0
            else:
                # expand buffer.
                new_size = tail + _buf_len
                if new_size < buf_size*2:
                    new_size = buf_size*2
                buf = <char*>realloc(buf, new_size)
                if buf == NULL:
                    # self.buf still holds old buffer and will be freed during
                    # obj destruction
                    raise MemoryError("Unable to enlarge internal buffer.")
                buf_size = new_size

        memcpy(buf + tail, <char*>(_buf), _buf_len)
        self.buf = buf
        self.buf_head = head
        self.buf_size = buf_size
        self.buf_tail = tail + _buf_len

    # prepare self.buf from file_like
    cdef fill_buffer(self):
        if self.file_like is not None:
            next_bytes = self.file_like_read(self.read_size)
            if next_bytes:
                self.append_buffer(PyBytes_AsString(next_bytes),
                                   PyBytes_Size(next_bytes))
            else:
                self.file_like = None

    cpdef unpack(self):
        """unpack one object"""
        cdef int ret
        while 1:
            _gc_disable()
            ret = template_execute(&self.ctx, self.buf, self.buf_tail, &self.buf_head)
            _gc_enable()
            if ret == 1:
                o = template_data(&self.ctx)
                template_init(&self.ctx)
                return o
            elif ret == 0:
                if self.file_like is not None:
                    self.fill_buffer()
                    continue
                raise StopIteration("No more unpack data.")
            else:
                raise ValueError("Unpack failed: error = %d" % (ret,))

    def __iter__(self):
        return self

    def __next__(self):
        return self.unpack()

