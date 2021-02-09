const express = require('express');
const router = express.Router();
const {sendGo, boards} = require('../OTA/server')
const multer = require("multer")

const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    cb(null, './files')
  },
  filename: function (req, file, cb) {
    cb(null, file.originalname + ".bin")
  }
})

const upload = multer({ storage: storage })


router.get("/clients", ((req, res) => {
    res.json(boards)
}))


router.post("/upload", upload.single("file"), (req, res) => {
  const group = req.body.group;
  const version = req.body.version;


  const groups = String(group).split(",");
  const all = groups.includes("all");
  for (let client of Object.values(boards)) {
    if (all || groups.includes(client.group)) {
      sendGo(client.address, client.port, version)
    }
  }

  res.json({message: "DONE!"})
});


router.get("/file/:version", function(req, res) {
  const fileName = (req.params.version)? req.params.version: "test.bin"
  const file = `${__dirname}/../files/${fileName}.bin`;
  res.download(file);
});

module.exports = router;
