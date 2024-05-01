let rootPath = process.argv[process.argv.length - 1];
let fs = require("fs");
let ws = require("ws");
let getHash = require("./hash");
let { createArrayBuffer, decodeArrayBuffer } = require("./ab");

let config = null;

let timeout = 5000;
let timerCheck = 0;
let running = false;

if (fs.existsSync("./autoOTAConfig.json")) {
    config = JSON.parse(fs.readFileSync("./autoOTAConfig.json").toString());
} else {
    config = {
        adminUserName: "",
        adminPassword: "",
        domain: "",
        port: 0,
        path: "",
        table: [
            //{
            //    appName: appName1,
            //    id: id1,
            //    blockSize: 8 * 1024,
            //    enable: true
            //},
            //{
            //    appName: appName2,
            //    id: id2,
            //    blockSize: 1 * 1024,
            //    enable: true
            //},
            //{
            //    appName: appName3,
            //    id: id3,
            //    blockSize: 16 * 1024,
            //    enable: true
            //},
            //...
        ]
    };
    fs.writeFileSync("./autoOTAConfig.json", JSON.stringify(config));
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

let firmwares = fs.readdirSync(rootPath + "firmware/");

let arr = [];

for (let i of firmwares) {
    let json = {
        fileName: i,
        meta: /(?<appName>.+)_(?<version>.+)_(?<hour>\d{2})\.(?<minute>\d{2})\.(?<second>\d{2})/.exec(i)
    };
    if (json.meta) {
        json.time = parseInt(fs.statSync(rootPath + "firmware/" + i).mtimeMs)
        json.appName = json.meta.groups.appName;
        arr.push(json);
    }
}

arr.sort((a, b) => {
    return b.time - a.time;
});

let target = null;

if(arr.length <1){
    return;
}

target = config.table.find(e => {
    return e.appName === arr[0].appName;
});

if (!target) {
    return;
}

if (!target.enable) {
    return;
}

let id = target.id;

if (!id || id.length != 64) {
    return;
}

target.fileName = arr[0].fileName;

console.log("Pending OTA firmware: ", target.fileName);

function sendOTARequest(client) {
    let userName = config.adminUserName;
    let password = config.adminPassword;

    userName = getHash(userName);
    password = getHash(password);

    let t = new Date().getTime();

    let hash = new Uint8Array(getHash(userName + password + t, !0));

    let firmware = new Uint8Array(fs.readFileSync(rootPath + "firmware/" + target.fileName));

    let firmwareHash = new Uint8Array(getHash(firmware, !0));

    let ab = createArrayBuffer(
        [
            0xAB,
            id,
            userName,
            firmware,
            t,
            hash,
            target.blockSize,
            firmwareHash
        ]
    );

    client.send(ab);
};

function pushFirmware() {
    if (!config.path.startsWith("/"))
        config.path = "/" + config.path;

    let client = new ws.WebSocket("ws://" + config.domain + ":" + config.port + config.path);
    client.on("open", function () {
        client.binaryType = "arraybuffer";
        console.log("Connected, uploading firmware...");

        sendOTARequest(client);

        timerCheck = setInterval(() => {
            if (!running) {
                sendOTARequest(client);
            } else {
                clearInterval(timerCheck);
            }
        }, timeout);
    });
    client.on("message", function (msg) {
        let arr = decodeArrayBuffer(msg);

        let command = arr[0];

        switch (command) {
            case 0x0c:
                this.send(createArrayBuffer([0xc0]));
                break;
            case 0xae:
                console.log("progress: " + arr[2]);
                running = true;
                break;
            case 0xfb:
                if (arr[3] == 100) {
                    console.log("OTA update finished!");
                    setTimeout(() => {
                        process.exit(0);
                    }, 100);
                }
                break;
            default:

        }
    });
    client.on("error", function (err) {
        console.log("Error OTA update :", err.toString());
        process.exit(1);
    });
};

pushFirmware();