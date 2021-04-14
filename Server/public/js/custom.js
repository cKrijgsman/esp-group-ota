let groups = [];

let alertCounter = 0;

(async function ($) {
    "use strict"; // Start of use strict

    $("#startUpload").on("click", (e) => {
        const data = new FormData();
        const version = $("#versionName").val()
        const g = $("#groupSelect").val();

        data.append("file", document.getElementById("file").files[0], version);
        data.append("group", g)
        data.append("version", version)

        if (g.length === 0) {
            return;
        }

        fetch('/api/upload', {
            method: 'POST',
            body: data
        }).then(res => res.json())
            .then(data => {
                console.log(data)
                document.getElementById("file").value = "";
            })

        // Stop submit action
        e.preventDefault();
    })

    $("#start-re-upload").on("click", (e) => {

        const g = $("#groupSelectReUpload").val();
        const v = $("#fileSelect").val()

        if (g.length === 0) {
            return;
        }


        fetch('/api/re-upload', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                group: g,
                version: v
            })
        }).then(res => res.json())
            .then(data => console.log(data))

        // Stop submit action
        e.preventDefault();
    })

    const webSocket = new WebSocket("ws://localhost:8080")

    webSocket.onmessage = (message) => {

        const temp = String(message.data)
        // We got boards
        if(temp.substr(0,2) === "B|"){
            const data = JSON.parse(temp.substr(2))
            const clients = data.Boards
            const groups = data.Groups
            updateViews(clients,undefined,undefined,groups)
        }

        //We got Alerts
        if(temp.substr(0,2) === "A|"){
            const alerts = JSON.parse(temp.substr(2))
            updateViews(undefined,undefined,alerts,undefined)
        }
    }

    // Get the client list on page load.
    const files = await getFiles();

    updateViews(undefined, files, []);
})(jQuery); // End of use strict

/**
 * Gets the clients from the server.
 * @returns {Promise<any>}
 */
async function getClients() {
    const res = await fetch("/api/clients", {
        method: 'GET'
    });
    return res.json();
}

async function getFiles() {
    const res = await fetch("/api/files", {
        method: 'GET'
    });
    return res.json();
}

async function getAlerts() {
    const res = await fetch("/api/alerts", {
        method: 'GET'
    });
    return res.json();
}

/**
 * Updates the selection view for picking groups to upload to.
 * @param clients - List of clients that is present.
 */
function updateGroups(clients) {
    let updated = false;

    const newGroups = Object.values(clients).map(x => x.group).filter((group, pos, self) => {
        return self.indexOf(group) === pos
    })

    // Check if there was an update in the groups
    if (groups.length !== newGroups.length) {
        updated = true
    } else {
        for (let group of newGroups) {
            if (groups.includes(group))
                continue;
            updated = true;
        }
    }

    groups = newGroups;

    if (updated) {
        const groupSelect = $("#groupSelect,#groupSelectReUpload")
        groupSelect.html("")
        groupSelect.append("<option value='all'>all</option>")
        for (let group of groups) {
            groupSelect.append(`<option value="${group}">${group}</option>`)
        }
    }

}

/**
 * Updates the client list view.
 * @param clients - Object of clients that is present.
 * @param files - The list of previously uploaded files.
 * @param alerts - The alerts generated on the server.
 * @param groups - Object containing all the groups
 */
function updateViews(clients, files, alerts, groups) {

    // Update the Client views
    if (typeof clients !== "undefined") {
        const clientDiv = $("#clientList");

        $("#connectedDevices").html(Object.values(clients).length)

        clientDiv.html("");


        for (let [key, value] of Object.entries(clients)) {
            const nameField = $(`<h4>${value.name}</h4>`)
            // Set name handler
            nameField.on('click', (e) => {
                const name = prompt("Provide board Name", "Jhon")
                if (!name)
                    return ;

                fetch("/api/name", {
                    method: "POST",
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        name: name,
                        client: key
                    })
                })

                e.preventDefault()
            })
            clientDiv.append(nameField)
            clientDiv.append(`<span class="tooltiptext">${key}</span>`)
            const groupBox = $(`
                <div>
                    <b>Group:</b> ${value.group} | <b>Version:</b> ${value.version}
                </div>
            `)
            groupBox.on('click', (e) => {
                const group = prompt("Provide group id between 1 and 255", "1")
                if (!group)
                    return

                fetch("/api/group", {
                    method: "POST",
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        group: group,
                        client: key
                    })
                })

                e.preventDefault()
            })
            clientDiv.append(groupBox)


        }

        // Update the Group views
        updateGroups(clients);
    }

    // Update the Client views
    if (typeof groups !== "undefined") {
        const groupsDiv = $("#groupList");

        groupsDiv.html("");


        for (let [key, value] of Object.entries(groups)) {
            const nameField = $(`<h4>${value.name}</h4>`)
            // Set name handler
            nameField.on('click', (e) => {
                const name = prompt("Provide group Name", "")
                if (!name || name === "")
                    return ;

                fetch("/api/group-name", {
                    method: "POST",
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        name: name,
                        group: key
                    })
                }).then(response => response.json()).then(data => {
                        if (data.message === "DONE!") {
                            nameField.text(name)
                        }
                    }
                )

                e.preventDefault()
            })
            groupsDiv.append(nameField)
            const groupBox = $(`
                <div>
                    <b>Group ID:</b> ${key} | <b>Current Version:</b> ${value.version} <br>
                    <b>Boards in group:</b> ${Object.values(value.boards).length}
                </div>
            `)
            groupsDiv.append(groupBox)


        }

        // Update the Group views
        updateGroups(clients);
    }

    // Update the Re-Upload file list
    if (typeof files !== "undefined") {
        const fileSelect = $("#fileSelect")
        fileSelect.html("");
        for (let file of files) {
            if (String(file).indexOf(".bin") !== -1) {
                file = String(file).split(".bin")[0];
                fileSelect.append(`<option value="${file}">${file}</option>`)
            }
        }
    }

    if (typeof alerts !== "undefined") {
        const counter = $("#alertCounter")

        alertCounter += alerts.length

        if (alertCounter > 0) {
            counter.text(alertCounter)
        }else {
            counter.text("")
        }

        const alertBox = $("#alertBox")
        for (let a of alerts) {
            console.log(a);
            let icon = '<div class="icon-circle bg-primary"><i class="fas fa-robot text-white"></i></div>'
            if (a.type === "update") {
                icon = '<div class="icon-circle bg-info"><i class="fas fa-pencil text-white"></i></div>'
            } else if (a.type === "warning") {
                icon = '<div class="icon-circle bg-warning"><i class="fas fa-warning text-white"></i></div>'
            }

            const currently = new Date(a.time);
            const element = $(`<a class="dropdown-item d-flex align-items-center" href="#"></a>`);
            element.append(`
            <div class="mr-3">
                ${icon}
            </div>
            <div>
                <div class="small text-gray-500">${currently.getHours() + ":"
            + currently.getMinutes() + ":"
            + currently.getSeconds()}</div>
                <span class="font-weight-bold">${a.message}</span>
            </div>`)

            element.on("click", () => {
                element.remove();
                alertCounter--;
                if (alertCounter > 0) {
                    counter.text(alertCounter)
                }else {
                    counter.text("")
                }
            });

            alertBox.append(element)
        }

    }
}
