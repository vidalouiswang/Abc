let rootPath = process.argv[process.argv.length - 1];
let fs = require("fs");
let ws = require("ws");
let getHash = require("./hash");
let { createArrayBuffer, decodeArrayBuffer } = require("./ab");

let config = null;

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
        let date = new Date("00:00:00 01/01/2200");
        date.setHours(parseInt(json.meta.groups.hour));
        date.setMinutes(parseInt(json.meta.groups.minute));
        date.setHours(parseInt(json.meta.groups.second));
        json.time = date.getTime();
        json.appName = json.meta.groups.appName;
        arr.push(json);
    }
}

arr.sort((a, b) => {
    return a.time - b.time;
});

let target = null;

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

function pushFirmware() {
    if (!config.path.startsWith("/"))
        config.path = "/" + config.path;

    let client = new ws.WebSocket("ws://" + config.domain + ":" + config.port + config.path);
    client.on("open", function () {
        client.binaryType = "arraybuffer";
        console.log("Connected, uploading firmware...");

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

        this.send(ab);
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