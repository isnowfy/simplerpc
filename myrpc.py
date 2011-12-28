import re
from marshal import loads,dumps
import gevent
from gevent import socket

remote_funcmap={}
def remote(func):
    funcname=func.__name__
    if not remote_funcmap.has_key(funcname):
        remote_funcmap[funcname]=func
    else:
        raise KeyError('%s:funcname declare more than once'%repr(funcname))
    return func

def query(query):
    funcname,args,kwargs=loads(query)
    func=remote_funcmap.get(funcname,None)
    if func:
        ret=func(*args,**kwargs)
        return ret
    else:
        raise Exception('FunctionNameError %s does not exist'%repr(funcname))

def handler(s,addr):
    while(True):
        data=s.recv(8192)
        if(data=='exit'):
            s.close()
            print 'exit'
            return
        try:
            ret=query(data)
            s.send(dumps({0:ret}))
        except Exception,e:
            s.send(dumps({1:str(e)}))

def main(port=12345):
    print remote_funcmap
    addr=('',port)
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.bind(addr)
    while True:
        s.listen(8192)
        ss,addr=s.accept()
        gevent.spawn(handler,ss,addr)
