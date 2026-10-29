import http.client
import os
import socket
import subprocess
import time
from pathlib import Path

base = Path(__file__).resolve().parents[1]
port = 18080
p = subprocess.Popen([str(base/'sws'), '-d', '-p', str(port), '-r', str(base/'www')], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    for _ in range(30):
        try:
            c = socket.create_connection(('127.0.0.1',port),.2)
            c.close()
            break
        except OSError:
            time.sleep(.1)
    def check(method,path,code,expected=None,body=None):
        conn = http.client.HTTPConnection('127.0.0.1',port,timeout=4)
        conn.request(method,path,body=body,headers={'Content-Type':'text/plain'})
        r=conn.getresponse(); data=r.read()
        assert r.status==code,(method,path,r.status,code,data)
        if expected is not None: assert expected in data,(method,path,data)
        if method=='HEAD': assert data==b''
        conn.close()
        print('PASS',method,path,code)
    check('GET','/',200,b'Simple Web Server')
    check('GET','/hello.txt',200,b'Hello from')
    check('HEAD','/hello.txt',200)
    check('GET','/missing',404)
    check('GET','/cgi-bin/hello.sh?name=test',200,b'Query: name=test')
    check('POST','/cgi-bin/hello.sh',200,b'POST data: abc',b'abc')
    check('POST','/hello.txt',405)
    check('GET','/../src/sws.c',403)
    print('All 8 checks passed')
finally:
    p.terminate()
    try: p.wait(timeout=3)
    except subprocess.TimeoutExpired: p.kill()
