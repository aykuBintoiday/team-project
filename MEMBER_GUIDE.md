# Thành viên 2 – HTTP_Static

**Phần chịu trách nhiệm:** `src/http.c, src/static.c`

## Bàn giao đã có
- Mã nguồn C cho module phụ trách; API liên kết với hai module còn lại.
- Đầy đủ source project để build và demo độc lập trên Ubuntu.
- Makefile, website mẫu, CGI script, test Python và tài liệu hướng dẫn.

## Cần tự thực hiện và xác minh
Kiểm tra GET, HEAD, HTTP/1.1 compatibility, MIME, 404, URL traversal và file ngoài document root.

## Cách chạy
```sh
make clean && make
make test
./sws -d -p 8080 -r www
```

Không xem đây là lịch sử đóng góp thực tế của thành viên; cần tự kiểm tra, sửa lỗi và thực hiện Git commit, review.
