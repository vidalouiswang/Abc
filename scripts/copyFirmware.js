let fs = require("fs");
let os = require("os");
console.log("copy firmware\n");

let slash = "/";
if (os.type() == "Windows_NT") {
    slash = "\\";
}

Date.prototype.getFormat = function (format) {
    let b = this;
    format = format || "yyyy年mm月dd日 hh:MM:ss";
    let year = b.getFullYear().toString();
    let month = (b.getMonth() + 1).toString();
    let date = b.getDate().toString();
    let hour = b.getHours().toString();
    let minute = b.getMinutes().toString();
    let second = b.getSeconds().toString();

    let fn = e => { return e.length == 1 ? "0" + e : e; };

    month = fn(month);
    date = fn(date);
    hour = fn(hour);
    minute = fn(minute);
    second = fn(second);

    fn = (reg, obj) => { format = format.replace(reg, obj); };

    fn(/yyyy/, year);
    fn(/mm/, month);
    fn(/dd/, date);
    fn(/hh/, hour);
    fn(/MM/, minute);
    fn(/ss/, second);

    return format;
};

let bootloaderPath = process.argv[process.argv.length - 1];
let envPath = bootloaderPath.match(/.+\/\.pio/i);//.toString();

if (!envPath) {
    envPath = bootloaderPath.match(/.+\\\.pio/i);
    if (!envPath) {
        console.log("Unknown error");
        return;
    }
}
envPath = envPath.toString();

envPath = envPath.substring(0, envPath.length - 4);

let firmwarePath = bootloaderPath.match(/.+\//i);
if (!firmwarePath) {
    firmwarePath = bootloaderPath.match(/.+\\/i);
    if (!firmwarePath) {
        console.log("Unknown error");
        return;
    }
}

let app_h, appName, appVersion;

if (fs.existsSync(envPath + "src/app/app.h")) {
    app_h = fs.readFileSync(envPath + "src/app/app.h").toString();
    appName = /#define\s+?APP_NAME\s+?(?<appName>.+)(\s|\n)?/g.exec(app_h).groups.appName;
    appName = appName.replace(/[\\\/,:;'"?\{\}\[\]~`&\^\*\r\n]/g, "");
    appVersion = /#define\s+APP_VERSION\s+?(?<appVersion>.+)(\s|\n)?/g.exec(app_h).groups.appVersion;
} else {
    appName = "Abc";
    appVersion = "10";
}

if (!fs.existsSync(bootloaderPath + "firmware/")) {
    fs.mkdirSync(bootloaderPath + "firmware/");
}

appVersion = appVersion.replace(/['"\\\/;:,\.]/gi,"_");


fs.copyFileSync(firmwarePath + 'firmware.bin',
    envPath + "firmware" + slash + appName + "_" + appVersion + "_" + new Date().getFormat("hh.MM.ss") + ".bin");

console.log("firmware copied\n");