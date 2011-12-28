/*
 * MessagePack for Python packing routine
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

#include <stddef.h>
#include <stdlib.h>
#include "sysdep.h"
#include "pack_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msgpack_packer {
    char *buf;
    size_t length;
    size_t buf_size;
} msgpack_packer;

typedef struct Packer Packer;

static inline int msgpack_pack_short(msgpack_packer* pk, short d);
static inline int msgpack_pack_int(msgpack_packer* pk, int d);
static inline int msgpack_pack_long(msgpack_packer* pk, long d);
static inline int msgpack_pack_long_long(msgpack_packer* pk, long long d);
static inline int msgpack_pack_unsigned_short(msgpack_packer* pk, unsigned short d);
static inline int msgpack_pack_unsigned_int(msgpack_packer* pk, unsigned int d);
static inline int msgpack_pack_unsigned_long(msgpack_packer* pk, unsigned long d);
static inline int msgpack_pack_unsigned_long_long(msgpack_packer* pk, unsigned long long d);

static inline int msgpack_pack_uint8(msgpack_packer* pk, uint8_t d);
static inline int msgpack_pack_uint16(msgpack_packer* pk, uint16_t d);
static inline int msgpack_pack_uint32(msgpack_packer* pk, uint32_t d);
static inline int msgpack_pack_uint64(msgpack_packer* pk, uint64_t d);
static inline int msgpack_pack_int8(msgpack_packer* pk, int8_t d);
static inline int msgpack_pack_int16(msgpack_packer* pk, int16_t d);
static inline int msgpack_pack_int32(msgpack_packer* pk, int32_t d);
static inline int msgpack_pack_int64(msgpack_packer* pk, int64_t d);

static inline int msgpack_pack_float(msgpack_packer* pk, float d);
static inline int msgpack_pack_double(msgpack_packer* pk, double d);

static inline int msgpack_pack_nil(msgpack_packer* pk);
static inline int msgpack_pack_true(msgpack_packer* pk);
static inline int msgpack_pack_false(msgpack_packer* pk);

static inline int msgpack_pack_array(msgpack_packer* pk, unsigned int n);

static inline int msgpack_pack_map(msgpack_packer* pk, unsigned int n);

static inline int msgpack_pack_raw(msgpack_packer* pk, size_t l);
static inline int msgpack_pack_raw_body(msgpack_packer* pk, const void* b, size_t l);

#define msgpack_pack_append_buffer(pk,data,l)\
    if ((pk)->length + (l) > (pk)->buf_size) {\
        (pk)->buf_size = ((pk)->length + (l)) * 2;\
        (pk)->buf = realloc((pk)->buf, (pk)->buf_size);\
        if(!(pk)->buf) return -1;\
    }\
    memcpy((pk)->buf + (pk)->length, (data), (l));\
    (pk)->length += (l);\
    return 0;

#define msgpack_pack_inline_func(name) \
	static inline int msgpack_pack ## name

#define msgpack_pack_inline_func_cint(name) \
	static inline int msgpack_pack ## name

#define msgpack_pack_user msgpack_packer*

#include "pack_template.h"


msgpack_packer *pk;

static inline int mypack(PyObject* o,int nest_limit,msgpack_packer* mypk){
    pk=mypk;
    return _pack(o,nest_limit);
}

inline int _pack(PyObject* o, int nest_limit){
    long long llval;
    unsigned long long ullval;
    long longval;
    double fval;
    char* rawval;
    int ret,n,i;

    if (nest_limit < 0)
        return -1;

    if (o == NULL || o == Py_None)
        ret = msgpack_pack_nil(pk);
    else if(o == Py_True)
            ret = msgpack_pack_true(pk);
    else if(o == Py_False)
            ret = msgpack_pack_false(pk);
    else if(PyLong_Check(o)){
        if(o > 0){
            ullval = PyLong_AsUnsignedLongLong(o);
            ret = msgpack_pack_unsigned_long_long(pk, ullval);
        }else{
            llval = PyLong_AsLongLong(o);
            ret = msgpack_pack_long_long(pk, llval);
        }
    } 
    else if(PyInt_Check(o)){
        longval = PyInt_AS_LONG((PyIntObject *)o);
        ret = msgpack_pack_long(pk, longval);
    }
    else if(PyFloat_Check(o)){
        fval=PyFloat_AsDouble(o);
        ret=msgpack_pack_double(pk,fval); 
    }
    else if(PyBytes_Check(o)){
        n=PyString_GET_SIZE(o);
        rawval = PyString_AS_STRING(o);
        ret = msgpack_pack_raw(pk, n);
        if(ret == 0)
            ret = msgpack_pack_raw_body(pk, rawval, n);
    }
    else if(PyUnicode_Check(o)){
        o = PyUnicode_AsUTF8String(o);
        n=PyString_GET_SIZE(o);
        rawval = PyString_AS_STRING(o);
        ret = msgpack_pack_raw(pk, n);
        if(ret == 0)
            ret = msgpack_pack_raw_body(pk, rawval, n);
    }
    else if(PyDict_Check(o)){
        n=PyDict_Size(o);
        ret = msgpack_pack_map(pk, n);
        if(ret == 0){
            Py_ssize_t pos;
            PyObject *key,*value;
            pos=0;
            while(PyDict_Next(o,&pos,&key,&value)){
                _pack(key,nest_limit-1);
                _pack(value,nest_limit-1);
            }
        }
    }
    else if(PySequence_Check(o)){
        n=PySequence_Length(o);
        ret = msgpack_pack_array(pk, n);
        if(ret == 0){
            for(i=0;i<n;++i){
                ret = _pack(PySequence_GetItem(o,i), nest_limit-1);
                if(ret != 0)
                    break;
            }
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

