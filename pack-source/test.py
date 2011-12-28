import mypack
import msgpack
import time

#s={123:(23,"werf",{12:"aaa"},(12,"weqe"))}
#s="asd21dasas"
z1=mypack.Packer()
z2=msgpack.Packer()
s=[i for i in range(100)]
tt=10000

a1=time.time()
for i in range(tt):
    tmp=z2.pack(s)
print "msgpack dumps:"+str(time.time()-a1)
a1=time.time()
for i in range(tt):
    ll=msgpack.unpackb(tmp)
print "msgpack loads:"+str(time.time()-a1)
a1=time.time()
for i in range(tt):
    tmp=z1.pack(s)
print "mypack dumps:"+str(time.time()-a1)
a1=time.time()
for i in range(tt):
    ll=mypack.unpackb(tmp)
print "mypack loads:"+str(time.time()-a1)
