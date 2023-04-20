import requests
from io import BytesIO

# 청크 데이터 생성
def chunked_data():
    chunks = [
        (b"ABCD", 4),
        (b"123", 3),
        (b"XYZ\n", 4),
        (b"456", 3)
    ]

    for data, size in chunks:
        yield f"{size:x}\r\n".encode()
        yield data
        yield b'\r\n'
    yield b'0\r\n\r\n'

data = BytesIO(b"".join(chunked_data()))

url = "http://localhost:4243/index/posttest.html"
headers = {
    "Transfer-Encoding": "chunked",
    "Content-Type": "application/octet-stream",
}

response = requests.post(url, headers=headers, data=data)

print(response.status_code)
print(response.text)
