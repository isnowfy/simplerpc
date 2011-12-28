import myclient
import time

def main():
    client=myclient.MyClient()
    a1=time.time()
    tmp=0
    for i in range(10000):
        tmp=client.add(y=1011230,x=212312345121200)
#        client.now()
#        client.add(1,2)
    print time.time()-a1
    print tmp
    client.ok()
    try:
        client.qq()
    except Exception,e:
        print str(e)
    client.close()

if __name__ == '__main__':
    main()
