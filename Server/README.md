#OTA SERVER

#prerequisites
This server runs on Node.js 1.12 and above. To run and install it, you will need both node and npm.
To install these, check out the website of node.js. https://nodejs.org/en/. \
For aditional suport check out https://docs.npmjs.com/downloading-and-installing-node-js-and-npm#using-a-node-installer-to-install-nodejs-and-npm.

# Setup
To install the servers dependencies, open the server location in a terminal and run the following command
```shell
npm install
```

On Windows it could be that this fails, if so it could be that you are missing the building tools essentials. \
if this is the case, follow this tutorial to fix that https://www.npmjs.com/package/windows-build-tools

# Running the server
To run the server, open a terminal on the location of the server and run the following command.
```shell
npm run start
```

The server should now indicate that it started listening for OTA devices.

To open the server page, go http://127.0.0.1:3000/ in a browser. You should now see the server.
