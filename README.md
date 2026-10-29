# CS631: Simple Web Server – Three-member integration release

**C/POSIX HTTP/1.0 teaching implementation**. Builds on Ubuntu 24.04; cross-platform compatibility with NetBSD 11.0 and OmniOS remains to be verified on those systems. The teacher's separate `sws(1)` manual, official checklist and class coding guidelines were not provided, so rubric compliance cannot yet be certified. Do not deploy publicly.

## Install, build and run

```sh
sudo apt update && sudo apt install -y build-essential python3 curl
make
./sws -d -p 8080 -r www
# In another terminal:
curl -i http://127.0.0.1:8080/
curl -I http://127.0.0.1:8080/hello.txt
curl -i 'http://127.0.0.1:8080/cgi-bin/hello.sh?name=test'
curl -i -d 'hello=world' http://127.0.0.1:8080/cgi-bin/echo.sh
make test
```

Without `-d`, the server forks into the background and suppresses standard output. To stop a foreground server press Ctrl+C, or `kill <PID>` for daemon mode. `make clean` deletes the executable.

## Ownership and integration

- Member 1: `src/server.c` – socket, concurrent fork, daemon, signals, CLI.
- Member 2: `src/http.c`, `src/static.c` – parse, file serving, HTTP status, MIME.
- Member 3: `src/cgi.c`, `www/cgi-bin/*`, `tests/*` – CGI and tests.
- Shared API: `src/sws.h`.

All ZIP copies are fully buildable standalone as a **complete collaborative server**, with the relevant member's ownership marked in `MEMBER_GUIDE.md`. Keep one canonical repository; do not pretend that copying the files is an actual individual's Git contribution.

## Restrictions / known limitations

- Limited HTTP/1.0 subset, no keep-alive/TLS/HTTP2.
- 16 KiB request-header limit, 1 MiB CGI POST-body limit, ~1 MiB CGI-output limit.
- CGI child output timeout is 10 seconds per read; high-volume production hardening is out of scope.
- CGI is enabled for executable paths under `/cgi-bin/` only.
- Path traversal via `..` is forbidden; realpath confines targets to document root. Race-free openat-based protection and permission sandboxing are not implemented.
- Advanced HTTP/CGI semantic corner cases and strict portability have not been certified.
- Real group username TAR, team Git log, school email and exact sws(1) CLI contract are pending real user materials.

See `checklist.md` for verification steps before grading.
