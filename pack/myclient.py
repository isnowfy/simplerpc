import mypack
from gevent import socket,Timeout

class MyClient(object):
    
    def __init__(self,host='localhost',port=12345):
        self._addr=(host,port)
        self._socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self._socket.connect(self._addr)
        self._pack=mypack.Packer()
        self._unpack=mypack.Unpacker()

    def __getattr__(self,funcname):
        if funcname.startswith('_'):
            return object.__getitem__(self,funcname)
        else:
            func=lambda *args,**kwargs:self.__call__(funcname,*args,**kwargs)
            func__name__='My.'+funcname
            return func

    def __call__(self,funcname,*args,**kwargs):
        query=(funcname,args,kwargs)
        with Timeout(5,Exception('timeout')):
            self._socket.send(self._pack.pack(query))
            while True:
                data=self._socket.recv(8192)
                self._unpack.feed(data)
                try:
                    ret=self._unpack.unpack()
                    break
                except Exception:
                    pass
        if ret:
            if not ret.get(1,None):
                return ret.get(0,None)
            else:
                raise Exception(ret[1])
        return None

    def close(self):
        self._socket.send('exit')
        self._socket.close()
