const dgram = require("dgram")
const client = dgram.createSocket('udp4');
const storage = require('node-persist');


/**
 * A Board is a representation of an ESP board
 * Every board is keyed on the mac address.
 *
 * @type {{ }}
 */
let boards = {};
let alerts = [];

const CLIENTS_PORT = 41222;

/**
 * Sends the go command to the client.
 * @param address - The IP address of the client.
 * @param version - The version of the file that the client should request.
 */
function sendGo(address, version) {
    console.log(CLIENTS_PORT);
    console.log(address);
    client.send(Buffer.from(`go|${version}`), CLIENTS_PORT, address, (err) => {
        if (err)
            console.error(err)
    })
}

function sendIdentify(address) {
    client.send(Buffer.from(`Identify!`), CLIENTS_PORT, address, (err) => {
        if (err)
            console.error(err)
    })
}

function setName(address,name) {
    client.send(Buffer.from(`nm|${name.toString().substr(0,64)}`), CLIENTS_PORT, address, (err) => {
        if (err)
            console.error(err)
    })
}

function setGroup(address,groupID) {
    client.send(Buffer.from(`g|${Number(groupID) % 255}`), CLIENTS_PORT, address, (err) => {
        if (err)
            console.error(err)
    })
}

/**
 * Removes the alert with the given id from the alert list.
 * @param alertId - the id of the alert to remove.
 */
function clearAlert(alertId) {
    alerts = alerts.filter(a => a.id !== alertId)
}

client.on('error', (err) => {
    console.log(`server error:\n${err.stack}`);
    server.close();
});

/**
 * Listens from the UDP messages send by the clients.
 */
client.on('message', (msg, rinfo) => {
    if (String(msg) !== "Polo")
        console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);

    // register new board
    if (msg.indexOf("|") !== -1) {
        const data = msg.toString().split("|")
        const mac = data[0].split(":").map((x) => Number(x).toString(16)).join(":");
        const name = data[1];
        const group = data[2];
        const version = data[3];

        const alert = {
            id: `${name}-${Date.now()}`,
            message: "",
            time: Date.now(),
            type: "new"
        }

        // Update alert message
        if (typeof boards[mac] === "undefined") {
            alert.message = `${name} just connected`
        } else {
            if (group !== boards[mac].group || version !== boards[mac].version) {
                alert.message = `${name} was updated.`;
                alert.type = "update";
            }
            if (name !== boards[mac].name) {
                alert.message = `${mac} updated his name from ${boards[mac].name} to ${name}`
                alert.type = "update";
            }
            if (rinfo.address !== boards[mac].address) {
                alert.message = `${name} switched from ${boards[mac].address} to ${rinfo.address}.`
                alert.type = "update";
            }
        }

        // Check if we stole the ip of an existing client.
        for (let [key, value] of Object.entries(boards)) {
            if (key === mac)
                continue;

            // Check if there was an other client with this IP
            if (value.address === rinfo.address) {
                // add alert
                const a = {
                    message: `${name} has claimed the ip of ${value.name}. ${value.name} must have disconnected!`,
                    time: Date.now(),
                    type: "warning"
                }
                alerts.push(a);
                // remove old client
                delete boards[mac];
            }
        }

        boards[mac] = {
            name: name,
            group: group,
            version: version,
            address: rinfo.address,
            port: rinfo.port,
            life: Date.now()
        }

        if (alert.message !== "") {
            alerts.push(alert)
        }
    }

    if (String(msg) === "Polo") {
        const matches = Object.values(boards).filter((board) => {
            return board.address === rinfo.address;
        })
        if (matches.length === 0) {
            // Board is not known ask to identify
            sendIdentify(rinfo.address)
        } else {
            // Update time of life.
            matches[0].life = Date.now();
        }
    }
});

client.on('listening', async () => {
    // Start the storage modual
    await storage.init({
        dir: '../resources/storage/',
        stringify: JSON.stringify,
        parse: JSON.parse,
        encoding: 'utf8',
        logging: false,  // can also be custom logging function
        ttl: false,
        expiredInterval: 2 * 60 * 1000, // every 2 minutes the process will clean-up the expired cache
        forgiveParseErrors: false
    });

    // Load known Boards
    const b = await storage.getItem('boards')
    if (b) {
        boards = b
    }

    // Announce that the server is awake
    const address = client.address();
    console.log(`server listening ${address.address}:${address.port}`);

    // Send the broadcast message.
    setInterval(function () {
        client.setBroadcast(true);
        client.send(Buffer.from(`Marco`), 41222, "255.255.255.255");
    }, 10000)
});

client.bind(41234);

module.exports = {sendGo, boards, alerts, clearAlert, setName, setGroup};
