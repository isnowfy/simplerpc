import re
import mypack
import gevent
from gevent import socket

pack=mypack.Packer()
unpack=mypack.Unpacker()
remote_funcmap={}
def remote(func):
    funcname=func.__name__
    if not remote_funcmap.has_key(funcname):
        remote_funcmap[funcname]=func
    else:
        raise KeyError('%s:funcname declare more than once'%repr(funcname))
    return func

def query(funcname,args,kwargs):
    func=remote_funcmap.get(funcname,None)
    if func:
        ret=func(*args,**kwargs)
        return ret
    else:
        raise Exception('FunctionNameError %s does not exist'%repr(funcname))

def handler(s,addr):
    while True:
        while True:
            data=s.recv(1024)
            if(data=='exit'):
                s.close()
                print 'exit'
                return
            unpack.feed(data)
            try:
                func,args,kwargs=unpack.unpack()
                break
            except Exception:
                pass
        try:
            ret=query(func,args,kwargs)
            send=pack.pack({0:ret})
        except Exception,e:
            send=pack.pack({1:str(e)})
        l=len(send)
        now=0
        while True:
            tmp=s.send(send[now:])
            if(tmp+now>=l):
                break
            now=now+tmp

def main(port=12345):
    print remote_funcmap
    addr=('',port)
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.bind(addr)
    while True:
        s.listen(8192)
        ss,addr=s.accept()
        gevent.spawn(handler,ss,addr)
