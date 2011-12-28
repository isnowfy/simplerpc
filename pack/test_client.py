import myclient

def main():
    client=myclient.MyClient()
    print client.add(1,2)
    print client.now()
    print client.add(1,2)
    client.ok([i for i in range(100)])
    try:
        client.err()
    except Exception,e:
        print str(e)
    client.close()

if __name__ == '__main__':
    main()
