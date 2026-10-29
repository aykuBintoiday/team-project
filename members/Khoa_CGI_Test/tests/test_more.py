import concurrent.futures, http.client, os, socket, subprocess, time
from pathlib import Path
base=Path(__file__).resolve().parents[1]
port=19080
proc=subprocess.Popen([str(base/'sws'),'-d','-p',str(port),'-r',str(base/'www')],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
def req(method,path,body=None,version=11):
 c=http.client.HTTPConnection('127.0.0.1',port,timeout=12)
 c._http_vsn=version;c._http_vsn_str='HTTP/1.0' if version==10 else 'HTTP/1.1'
 c.request(method,path,body=body,headers={'Content-Type':'text/plain'})
 r=c.getresponse();b=r.read();answer=(r.status,r.version,b,dict(r.getheaders()));c.close();return answer
try:
 for _ in range(50):
  try:socket.create_connection(('127.0.0.1',port),.1).close();break
  except OSError:time.sleep(.1)
 tests=[('HTTP/1.0 GET',lambda: req('GET','/',version=10)[0]==200),
 ('HTTP/1.1 accepted',lambda: req('GET','/')[1]==10),
 ('missing file',lambda: req('GET','/no-such-file')[0]==404),
 ('HEAD no body',lambda: req('HEAD','/hello.txt')[2]==b''),
 ('POST static rejected',lambda: req('POST','/hello.txt',b'hi')[0]==405),
 ('CGI query',lambda: b'foo=bar' in req('GET','/cgi-bin/echo.sh?foo=bar')[2]),
 ('CGI POST',lambda: b'a=b' in req('POST','/cgi-bin/echo.sh',b'a=b')[2]),
 ('large CGI POST',lambda: len(req('POST','/cgi-bin/echo.sh',b'x'*131072)[2])>131072),
 ('URI traversal rejected',lambda: req('GET','/../src/server.c')[0] in (403,404)),
 ('50 concurrent GET requests',lambda: all(x==200 for x in concurrent.futures.ThreadPoolExecutor(max_workers=15).map(lambda _:req('GET','/')[0],range(50)))),
 ]
 failed=[]
 for title,fn in tests:
  try: ok=fn()
  except Exception as ex:ok=False;print('EXCEPTION',title,repr(ex))
  print('PASS' if ok else 'FAIL',title)
  if not ok:failed.append(title)
 print(f'{len(tests)-len(failed)}/{len(tests)} extended checks passed')
 if failed:raise SystemExit(1)
finally:
 proc.terminate()
 try:proc.wait(timeout=4)
 except subprocess.TimeoutExpired:proc.kill()
