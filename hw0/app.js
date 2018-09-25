const express = require("express");
const app = express();
const bodyParser = require("body-parser");
const childProcess = require("child_process");

app.listen(8000, () => {
  console.log("Server start listening at 8000....");
});

app.use(
  bodyParser.urlencoded({
    extended: true
  })
);
app.use(bodyParser.json());

app.use("/", express.static("./public"));

app.set("view engine", "ejs");

app.get("/search", (req, res) => {
  let userInput = req.query.search;
  let data = [];

  console.log(userInput);

  childProcess.exec(
    `grep ${userInput} ./final_data.txt`,
    { maxBuffer: 32 * 10000000 },
    (err, stdout) => {
      if (err) {
        console.log(err);
        return;
      }
      let result = stdout.split("\n");
      result.forEach(line => {
        let token = line.split("#");
        if (token[0].indexOf(userInput) != -1) {
          data.push({
            context: token[0],
            title: token[1].slice(3),
            url: token[2].slice(3)
          });
        }
      });

      res.render("search.ejs", { data: data });
      res.end();
    }
  );
});
