let rootPath = process.argv[process.argv.length - 1];
let fs = require("fs");
let ws = require("ws");

let config = JSON.parse(fs.readFileSync("./config.json").toString());

if (config.usingLANOTA) {
    let client = new ws.WebSocket("ws://" + config.localIP + ":" + config.localPort);
    client.on("open", function () {
        console.log("Connected");
    });
    client.on("message",function(msg){
        
    });

}