# Hướng dẫn ghép code của 3 thành viên

1. Tất cả cài `build-essential`, `make`, `git`, `python3`, `curl` trên Ubuntu.
2. Bạn 1 quản lý `server.c`, bạn 2 quản lý `http.c` và `static.c`, bạn 3 quản lý `cgi.c`, CGI scripts và tests.
3. Thống nhất header `sws.h`. Không đổi chữ ký hàm khi chưa báo cả nhóm.
4. Trong repo chung, copy đúng file từ từng ZIP vào vị trí tương ứng. Chạy `make clean && make && make test`.
5. Mỗi bạn tạo branch và commit phần mình tự kiểm thử/sửa. Đừng sử dụng Git commit giả mạo.
6. Đọc tài liệu `sws(1)` chính thức rồi bổ sung các chức năng bắt buộc; kiểm thử trên NetBSD/OmniOS trước khi nộp.
7. Duyệt code bảo mật, test đồng thời, malformed request, CGI output, permissions. Không phơi bày demo ra Internet.
