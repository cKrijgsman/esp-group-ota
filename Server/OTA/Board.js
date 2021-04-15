class Board {
    address;

    /**
     * Creates a board instance.
     * @param {string} mac - The mac address of the board
     * @param {number} group - The group number of a board (between 0 and 1)
     * @param {string} version - the version of the file that is on the board.
     * @param {string} name - the name of the board
     * @param address
     * @param {Socket} udpClient - the udp client to send message from
     * @param {number} udpPort
     */
    constructor(mac, group, version, name, address, udpClient, udpPort) {
        this.address = address;
        this.mac = mac;
        this.group = group;
        this.version = version;
        this.name = name;

        this.udpClient = udpClient
        this.udpPort = udpPort;

        this.online = true;

        this.lastActiveTime = Date.now();
    }

    updateVersion(version) {
        if (version !== this.version) {
            // Send a go to the board
            console.log(`go|${version}`)
            this.udpClient.send(Buffer.from(`go|${version}`), this.udpPort, this.address, (err) => {
                if (err)
                    console.error(err)
            })
        }
    }

    checkAlive() {
        if (this.online && this.lastActiveTime + 30000 < Date.now()) {
            this.online = false
            return 0
        } else if (this.lastActiveTime + 30000 > Date.now() && !this.online) {
            this.online = true
            return 1
        }
        return 2
    }
}

module.exports = Board
