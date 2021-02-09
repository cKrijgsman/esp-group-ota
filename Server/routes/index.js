const express = require('express');
const router = express.Router();
const {sendGo, testConnection,boards} = require('../OTA/server')

router.get("/clients", ((req, res) => {
    res.json(boards)
}))

/* GET home page. */
router.get("/go", (req, res) => {
  const address = (req.query.address)? req.query.address : "localhost";
  const port = (req.query.port)? req.query.port : "41234";

  sendGo(address,port);

  res.send("Going!");
})

router.get("/test", (req, res) => {
  testConnection();
  res.send("test")
})

router.get("/file", function(req, res) {
  const fileName = (req.query.fileName)? req.query.fileName: "test"
  const file = `${__dirname}/../files/${fileName}/${fileName}.ino`;
  res.download(file);
});

module.exports = router;
