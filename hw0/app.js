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
  const sendOutputWithUserInputOnHead = userInput => {
    childProcess.exec(
      `grep ^${userInput} ./final_data.txt`,
      { maxBuffer: 32 * 10000000 },
      (err, stdout) => {
        if (err) {
          console.log(err);
          return;
        }

        let result = stdout.split("\n");
        let data = [];
        let pageCount = 0;
        const COUNT_OF_ONE_PAGE = 100;
        let totalResultCount = 0;

        result.forEach(line => {
          if (line) {
            let token = line.split("\t");
            data.push({
              context: token[0],
              title: token[1].slice(3),
              url: token[2].slice(3)
            });
            totalResultCount += 1;
          }
        });

        res.render("search.ejs", {
          data: data,
          totalResultCount: totalResultCount
        });
        res.end();
      }
    );
  };

  const sendGeneralOutput = userInput => {
    childProcess.exec(
      `grep ${userInput} ./final_data.txt`,
      { maxBuffer: 32 * 10000000 },
      (err, stdout) => {
        if (err) {
          console.log(err);
          return;
        }

        let result = stdout.split("\n");
        let data = [];
        let totalResultCount = 0;

        result.forEach(line => {
          if (line) {
            let token = line.split("\t");

            if (token[0].indexOf(userInput) !== -1) {
              data.push({
                context: token[0],
                title: token[1].slice(3),
                url: token[2].slice(3)
              });
              totalResultCount += 1;
            }
          }
        });
        res.render("search.ejs", {
          data: data,
          totalResultCount: totalResultCount
        });
        res.end();
      }
    );
  };

  let userInput = req.query.search;

  console.log(userInput);

  if (userInput.charAt(0) === "#") {
    sendOutputWithUserInputOnHead(userInput.slice(1));
  } else {
    sendGeneralOutput(userInput);
  }
});
