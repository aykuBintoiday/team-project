# CS631 submission checklist (project draft; original instructor checklist not supplied)

- [x] C program, POSIX TCP socket server
- [x] HTTP/1.0 responses; accepts HTTP/1.1 requests
- [x] Fork concurrent connections and SIGCHLD reaping
- [x] Background daemon by default; foreground debug `-d`
- [x] Static GET/HEAD, CGI GET/POST
- [x] Common HTTP status codes
- [x] Relative URI path checks, document-root confinement
- [x] Makefile, README, runnable tests
- [ ] Compare exact command-line options and semantics against instructor sws(1) man page
- [ ] Confirm instructor-specific required edge cases, syntax and CGI contract
- [ ] Build and test on NetBSD 11.0 reference environment
- [ ] Build and test on OmniOS reference environment
- [ ] Record actual Git commit history and member contributions
- [ ] Rename TAR using real group usernames and submit from school account

Security note: educational server, not approved for public internet exposure.
