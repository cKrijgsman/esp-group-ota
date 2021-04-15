// Call the dataTables jQuery plugin
$(document).ready(function () {
    $('#dataTable').DataTable({
        "proccessing": true,
        "ajax": {
            "url": "./api/client-list",
            "dataSrc": ""
        },
        "columns": [
            {
                "data": null,
                "defaultContent" : "<span style='margin-top: 4px' class='dot client-active'>Active</span>",
                "width": "5%",
                "className": "dt-center"
            },
            {"data": "name"},
            {"data": "group"},
            {"data": "version"},
            {"data": "address"},
            {"data": "mac"},
            {
                "data": null,
                "defaultContent": "<button class='btn'><i class='fa fa-pen'></i></button>",
                "width": "5%",
                "className": "dt-center"
            }
        ]
    });
});

