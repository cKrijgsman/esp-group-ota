const dgram = require("dgram")
const client = dgram.createSocket('udp4');

const boards = {};

// Send hello
function sendGo(address, port, version) {
  client.send(Buffer.from(`go|${version}`), port, address, (err) => {
    if (err)
      console.error(err)
  })
}

client.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

client.on('message', (msg, rinfo) => {
  console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);
  // register new board
  if (msg.indexOf("|") !== -1) {
    const data = msg.toString().split("|")
    const mac = data[0].split(":").map((x) => Number(x).toString(16)).join(":");
    const name = data[1];
    const group = data[2];
    const version = data[3];

    // TODO Check if board was already known
    boards[mac] = {
      name: name,
      group: group,
      version: version,
      address: rinfo.address,
      port: rinfo.port
    }
  }
});

client.on('listening', () => {
  const address = client.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

client.bind(41234);

module.exports = {sendGo, boards};
