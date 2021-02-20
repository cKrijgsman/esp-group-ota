let groups = [];

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

    // Check for clients
    $("#reloadClients").on("click", async () => {
        const clients = await getClients();

        updateViews(clients);
    })


    // Get the client list on page load.
    const clients = await getClients();
    const files = await getFiles();
    const alerts = await getAlerts();

    updateViews(clients, files, alerts);
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

    console.log(newGroups);

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
 * @param clients - List of clients that is present.
 * @param files - The list of previously uploaded files.
 * @param alerts - The alerts generated on the server.
 */
function updateViews(clients, files, alerts) {

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
                fetch('/api/remove-alert', {
                    method: 'POST',
                    body: JSON.stringify({alertId: a.id})
                })
                element.remove();
            });

            alertBox.append(element)
        }
    }
}
