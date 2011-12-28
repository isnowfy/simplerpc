import myrpc
import time
import gevent
import math

@myrpc.remote
def add(x,y):
    return x+y

@myrpc.remote
def now():
    return time.strftime('%Y-%m-%d %H:%M:%S')

@myrpc.remote
def ok(x):
    print x

@myrpc.remote
def loop():
    gevent.sleep(20)

@myrpc.remote
def err():
    raise Exception('haha')

if __name__ == '__main__':
    myrpc.main()
