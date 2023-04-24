#!/usr/bin/env python3

import os
import cgi
import cgitb

# 에러 추적을 활성화
cgitb.enable()

# CGI 스크립트의 헤더 출력
# print("Content-Type: text/html")
# print()

def main():
    # CONTENT_LENGTH 환경 변수 가져오기
    # content_length = int(os.environ.get('CONTENT_LENGTH', 0))

    # Query String 처리 (예: 이름)
    # query_string = os.environ.get('QUERY_STRING', '')
    # print(query_string)
    # query_params = dict(qc.split('=') for qc in query_string.split('&'))
    # print(query_params)

    query_params = {}
    # while (1):
    #     continue
    # if query_string:
    #   query_params = dict(qc.split('=') for qc in query_string.split('&'))


    name = query_params.get('name', 'default')
    age = query_params.get('age', '0')
    # print("Content-Type: text/html")

    # POST 데이터 읽기
    # post_data = None
    # if content_length > 0:
    #     post_data = os.sys.stdin.buffer.read(content_length)

    # 폼 데이터 처리 (예: 이름)

    # HTML 출력
    # print(f"<html>")
    # print(f"<head><title>Python CGI POST Example with CONTENT_LENGTH</title></head>")
    # print(f"<body>")
    # print(f"<h1>Hello, {name}!</h1>")
    # print(f"<p>Age: {age}</p>")
    # if post_data:
    #     print(f"<p>Raw POST data: {post_data.decode()}</p>")
    # print(f"</body>")
    # print(f"</html>")

if __name__ == "__main__":
    main()
