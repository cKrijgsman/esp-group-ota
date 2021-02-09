let groups = [];

(async function ($) {
  "use strict"; // Start of use strict

  $("#startUpload").on("click", (e) => {
    const data = new FormData();
    data.append("file", document.getElementById("file").files[0], $("#versionName").val());
    data.append("group", $("#groupSelect").val())
    data.append("version", $("#versionName").val())

    fetch('/upload', {
      method: 'POST',
      body: data
    }).then(res => res.json())
      .then(data => console.log(data))

    // Stop submit action
    e.preventDefault();
  })

  // Check for clients
  $("#reloadClients").on("click", async () =>{
    const clients = await getClients();

    updateViews(clients);
    updateGroups(clients);
  })


  // Get the client list on page load.
  const clients = await getClients();

  updateViews(clients);
  updateGroups(clients);
})(jQuery); // End of use strict

/**
 * Gets the clients from the server.
 * @returns {Promise<any>}
 */
async function getClients() {
  const res = await fetch("/clients", {
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

  const newGroups = Object.values(clients).map(x => x.group)

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
    const groupSelect = $("#groupSelect")
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
 */
function updateViews(clients) {
  const clientDiv = $("#clientList");

  clientDiv.html("");
  for (let [key, value] of Object.entries(clients)) {
    clientDiv.append(`
    <h4>${value.name}</h4>
    <span class="tooltiptext">${key}</span>
    <div>
        <b>Group:</b> ${value.group} | <b>Version:</b> ${value.version}
    </div>
  `)
  }

}