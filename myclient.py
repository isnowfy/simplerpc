from marshal import loads,dumps
from gevent import socket,Timeout

class MyClient(object):
    
    def __init__(self,host='localhost',port=12345):
        self._addr=(host,port)
        self._socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self._socket.connect(self._addr)

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
            self._socket.send(dumps(query))
            ret=self._socket.recv(8192)
        if ret:
            tmp=loads(ret)
            if not tmp.get(1,None):
                return tmp.get(0,None)
            else:
                raise Exception(tmp[1])
        return None

    def close(self):
        self._socket.send('exit')
        self._socket.close()
