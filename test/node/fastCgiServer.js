const fastcgi = require("node-fastcgi");

// FastCGI 요청 처리 함수
function onRequest(request, response) {
  (cout << "helloword!!!!!!!!!!!!!!!") << endl;
  response.writeHead(200, {
    "Content-Type": "text/plain",
  });
  response.end("Hello, World!");
}

// FastCGI 서버 시작
fastcgi.createServer(onRequest).listen(9000, () => {
  console.log("FastCGI server is running on port 9000");
});
