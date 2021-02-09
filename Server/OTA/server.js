const dgram = require("dgram")
const client = dgram.createSocket('udp4');

const boards = {};

// Send hello
function sendGo(address, port) {
  client.send(Buffer.from("Go!"), port, address, (err) => {
    if (err)
      console.error(err)
  })
}

function testConnection() {
  client.send(Buffer.from("test|a|1"), 41234, "localhost", (err) => {
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
    const name = data[0]
    const group = data[1]
    const version = data[2]

    // TODO Check if board was already known
    boards[name] = {
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

module.exports = {sendGo, testConnection, boards};
