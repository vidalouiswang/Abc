let rootPath = process.argv[process.argv.length - 1];
let fs = require("fs");
let ws = require("ws");

let config = null;

if (fs.existsSync("./config.json")) {
    JSON.parse(fs.readFileSync("./config.json").toString());
} else {
    config = {
        adminUserName: "",
        adminPassword: "",
        domain: "",
        port: 0,
        path: "",
        table: [
            //{
            //    nickname1: id1,
            //    blockSize: 8 * 1024
            //},
            //{
            //    nickname2: id2,
            //    blockSize: 1 * 1024
            //},
            //{
            //    nickname3: id3,
            //    blockSize: 16 * 1024
            //},
            //...
        ]
    };
    fs.writeFileSync("./config.json", JSON.stringify(config));
}

if (
    !config.adminUserName.length ||
    !config.adminPassword.length ||
    !config.domain.length ||
    !config.port ||
    !config.table.length
) {
    console.log("No OTA basic information");
    return;
}

let files = fs.readdirSync(rootPath + "app");
if (!files.length) {
    return;
}

let pattern = /(?<appName>.+)_(?<appVersion>.+)_(?<compileTime>.+)\.bin/;

let appName = fs.readFileSync(rootPath + "src/app/app.h").toString();
appName = /#define\s+?APP_NAME\s+?(?<appName>.+)(\s|\n)?/g.exec(appName).groups.appName;
appName = appName.replace(/[\\\/,:;'"?\{\}\[\]~`&\^\*\r\n]/g, "");

let target = null;

for (let i of files) {
    let name = pattern.exec(i);
    if (name && name.groups) {
        if (name.groups.appName === appName) {
            target = i;
            break;
        }
    }
}

if (!target) {
    return;
}

let id = null;
id = config.table[appName];

if (!id || id.length != 64) {
    return;
}


function pushFirmware() {
    let client = new ws.WebSocket("ws://" + config.domain + ":" + config.port + config.path);
    client.on("open", function () {
        console.log("Connected");
    });
    client.on("message", function (msg) {

    });
    client.on("error", function (err) {
        console.log("Error OTA update :", err.toString());
        process.exit(1);
    });
};
