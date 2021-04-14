class Group {
    constructor(id,name, version) {
        this.id = id
        this.name = name
        this.version = version

        this.boards = {}
    }

    hasBoard(board) {
        return typeof this.boards[board.mac] !== "undefined"
    }

    addBoard(board) {
        if (this.hasBoard(board)) {
            // Remove the old board
            delete this.boards[board.mac]
        }
        this.boards[board.mac] = board
    }

    getBoard(board) {
        return this.boards[board.mac]
    }

    remove(board) {
        if (this.hasBoard(board)) {
            // Remove the old board
            delete this.boards[board.mac]
        }
    }
}

module.exports = Group
