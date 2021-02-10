const express = require('express');
const router = express.Router();
const {sendGo, boards, alerts, clearAlert} = require('../OTA/server')
const multer = require("multer")
const fs = require("fs")

const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    cb(null, './files')
  },
  filename: function (req, file, cb) {
    cb(null, file.originalname + ".bin")
  }
})

const upload = multer({storage: storage})


/**
 * Returns a list of all the connected clients.
 */
router.get("/clients", ((req, res) => {
  res.json(boards)
}))

/**
 * Returns a list of al the previously uploaded files.
 */
router.get("/files", ((req, res) => {
  fs.readdir(`${__dirname}/../files/`, (err, files) => {
    if (err) {
      console.error(err)
      res.status(500);
      res.send("Error reading dir")
    }
    res.json(files)
  })
}))

/**
 * Send the alerts to the client.
 */
router.get("/alerts", ((req, res) => {
  const newAlerts = []
  for (let i = 0; i < alerts.length; i++) {
    newAlerts.push(alerts[i])
  }

  res.json(newAlerts);
}))

router.post("/remove-alert", (req, res) => {
  const alertId = req.body.alertId;

  clearAlert(alertId);

  res.send("alert removed.")
})

/**
 * Upload a file and sends the go to all the selected clients.
 */
router.post("/upload", upload.single("file"), (req, res) => {
  const group = req.body.group;
  const version = req.body.version;


  sendToClients(group, version)

  res.json({message: "DONE!"})
});

/**
 * Uses and already uploaded file and sends the go to all the selected clients.
 */
router.post("/re-upload", (req, res) => {
  const group = req.body.group;
  const version = req.body.version;

  sendToClients(group, version)


  res.json({message: "DONE!"})
});

/**
 * Sends the Go command to all the clients that fit the group list.
 * @param group - The groups to send the go to.
 * @param version - The clients that are connected.
 */
function sendToClients(group, version) {
  const groups = String(group).split(",");
  const all = groups.includes("all");
  for (let client of Object.values(boards)) {
    if (all || groups.includes(client.group)) {
      console.log(`Sending go to ${client.name}`)
      sendGo(client.address, version)
    }
  }
}

/**
 * Get the file.
 * Called by the client when the need the file.
 */
router.get("/file/:version", function (req, res) {
  const fileName = (req.params.version) ? req.params.version : "test.bin"
  const file = `${__dirname}/../files/${fileName}.bin`;
  res.download(file);
});

module.exports = router;
