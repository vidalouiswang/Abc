let getHash = require("./hash");
let fs = require("fs");

let rootPath = process.argv[process.argv.length - 1];

console.log("replacing AP index.html\n");

let html = fs.readFileSync(rootPath + "ap/out.txt").toString();

let lastHash = "";
let currentHash = getHash(html);

if (fs.existsSync("./.htmlHash.txt")) {
    lastHash = fs.readFileSync("./.htmlHash.txt").toString();
}
fs.writeFileSync("./.htmlHash.txt", currentHash);

let headerPath = rootPath + "lib/globalmanager/globalmanager.h";


function replace() {
    console.log("AP index.html modified, update header file");

    let globalmanager_h = fs.readFileSync(headerPath).toString();

    let date = new Date().toString();

    globalmanager_h = globalmanager_h.replace(/\/\/\s*?apindex[\S\s]+\/\/\s*?apindex/g,
        `//apindex ${date}\r\nconst char* serverIndex = R"(${html})";\r\n//apindex`);



    fs.writeFileSync(headerPath, globalmanager_h);

    console.log("replace html OK\n");
};

function fillCompileTime() {
    let globalmanager_h = fs.readFileSync(headerPath).toString();

    let date = new Date();

    let M = (date.getMonth() + 1);
    M = M < 10 ? "0" + M : M;

    let d = date.getDate();
    d = d < 10 ? "0" + d : d;

    let h = date.getHours();
    h = h < 10 ? "0" + h : h;

    let m = date.getMinutes();
    m = m < 10 ? "0" + m : m;

    let s = date.getSeconds();
    s = s < 10 ? "0" + s : s;

    let dateString = `${date.getFullYear()}/${M}/${d}-${h}:${m}:${s}`;

    //replace firmware compile time
    globalmanager_h = globalmanager_h.replace(/#define\s*?firmware_compile_time\s*?.+\s/gi, `#define FIRMWARE_COMPILE_TIME "@${dateString}"\r\n`);

    fs.writeFileSync(headerPath, globalmanager_h);
};


if (lastHash.length) {
    if (lastHash != currentHash) {
        replace();
    } else {
        console.log("html doesn't change");
    }

} else {
    replace();
}

fillCompileTime();