import myclient

def main():
    client=myclient.MyClient()
    print client.add(1,2)
    print client.now()
    print client.add(1,2)
#    client.ok([i for i in range(100)])
    try:
        client.err()
    except Exception,e:
        print str(e)


    a=[i+30027671 for i in range(200000)]
    b=client.echo(a)
    print b[-1]
    client.close()

if __name__ == '__main__':
    main()
