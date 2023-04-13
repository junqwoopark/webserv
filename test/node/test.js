const express = require("express");
const app = express();
const port = 9000;

app.get("/hello", (req, res) => {
  console.log("helloword!!!!!!!!!!!!!!!");
  res.send("Hello World!");
});

app.use((req, res, next) => {
  console.log("notfound!!!!!!!!!!!!!!!");
  res.status(404).send("404 Not Found");
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
});
