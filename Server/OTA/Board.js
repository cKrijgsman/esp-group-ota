class Board {
    /**
     * Creates a board instance.
     * @param {string} mac - The mac address of the board
     * @param {number} group - The group number of a board (between 0 and 1)
     * @param {string} version - the version of the file that is on the board.
     * @param {string} name - the name of the board
     */
    constructor(mac, group, version, name) {
        this.mac = mac;
        this.group = group;
        this.version = version;
        this.name = name;

        this.lastActiveTime = Date.now();
    }
}

module.exports = Board
