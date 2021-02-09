const dgram = require("dgram")
const client = dgram.createSocket('udp4');

// Send hello
function sendGo(address, port) {
  client.send(Buffer.from("Go!"), port, address, (err) => {
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
});

client.on('listening', () => {
  const address = client.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

client.bind(41234);

module.exports = {sendGo};
