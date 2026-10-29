# Hướng dẫn ghép nhóm

1. Cả ba ZIP có đầy đủ mã nguồn chạy thử; khác nhau ở MEMBER_GUIDE.md và test/module sở hữu.
2. Tạo một repository chung từ mã nguồn của bản tích hợp.
3. Bạn 1 quản lý src/server.c, bạn 2 quản lý src/http.c và src/static.c, bạn 3 quản lý src/cgi.c, www/cgi-bin và tests.
4. Mỗi người đọc hiểu, tự sửa đổi, kiểm thử, tự commit phần việc thực tế.
5. Cùng sửa và review các thay đổi ở src/sws.h.
6. Chạy `make clean && make && make test` sau mỗi pull request.
7. Trước khi nộp: đối chiếu tài liệu sws(1) thực tế, kiểm thử NetBSD và OmniOS, bổ sung Git log và TAR tên username.
