/*
 * MessagePack for Python unpacking routine
 *
 * Copyright (C) 2009 Naoki INADA
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#define MSGPACK_MAX_STACK_SIZE  (1024)
#include "unpack_define.h"

#define msgpack_unpack_struct(name) \
	struct template ## name

#define msgpack_unpack_func(ret, name) \
	static inline ret template ## name

#define msgpack_unpack_callback(name) \
	template_callback ## name

#define msgpack_unpack_object PyObject*


struct template_context;
typedef struct template_context template_context;

static inline int template_callback_uint16(uint16_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyInt_FromLong((long)d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}
static inline int template_callback_uint8(uint8_t d, msgpack_unpack_object* o)
{
    return template_callback_uint16(d, o);
}


static inline int template_callback_uint32(uint32_t d, msgpack_unpack_object* o)
{
    PyObject *p;
    if (d > LONG_MAX) {
        p = PyLong_FromUnsignedLong((unsigned long)d);
    } else {
        p = PyInt_FromLong((long)d);
    }
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_uint64(uint64_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyLong_FromUnsignedLongLong(d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_int32(int32_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyInt_FromLong(d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_int16(int16_t d, msgpack_unpack_object* o)
{
    return template_callback_int32(d, o);
}

static inline int template_callback_int8(int8_t d, msgpack_unpack_object* o)
{
    return template_callback_int32(d, o);
}

static inline int template_callback_int64(int64_t d, msgpack_unpack_object* o)
{
    PyObject *p = PyLong_FromLongLong(d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_double(double d, msgpack_unpack_object* o)
{
    PyObject *p = PyFloat_FromDouble(d);
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_float(float d, msgpack_unpack_object* o)
{
    return template_callback_double(d, o);
}

static inline int template_callback_nil(msgpack_unpack_object* o)
{ Py_INCREF(Py_None); *o = Py_None; return 0; }

static inline int template_callback_true(msgpack_unpack_object* o)
{ Py_INCREF(Py_True); *o = Py_True; return 0; }

static inline int template_callback_false(msgpack_unpack_object* o)
{ Py_INCREF(Py_False); *o = Py_False; return 0; }

static inline int template_callback_array(unsigned int n, msgpack_unpack_object* o)
{
    PyObject *p = PyTuple_New(n);
    //PyObject *p = u->use_list ? PyList_New(n) : PyTuple_New(n);

    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_array_item(unsigned int current, msgpack_unpack_object* c, msgpack_unpack_object o)
{
   // PyList_SET_ITEM(*c, current, o);
    PyTuple_SET_ITEM(*c, current, o);
    return 0;
}

static inline int template_callback_map(unsigned int n, msgpack_unpack_object* o)
{
    PyObject *p = PyDict_New();
    if (!p)
        return -1;
    *o = p;
    return 0;
}

static inline int template_callback_map_item(msgpack_unpack_object* c, msgpack_unpack_object k, msgpack_unpack_object v)
{
    if (PyDict_SetItem(*c, k, v) == 0) {
        Py_DECREF(k);
        Py_DECREF(v);
        return 0;
    }
    return -1;
}

static inline int template_callback_raw(const char* b, const char* p, unsigned int l, msgpack_unpack_object* o)
{
    PyObject *py;
//  py = PyUnicode_Decode(p, l, u->encoding, u->unicode_errors);
    py = PyBytes_FromStringAndSize(p, l);
    if (!py)
        return -1;
    *o = py;
    return 0;
}

#include "unpack_template.h"
