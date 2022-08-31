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


function replace() {
    console.log("AP index.html modified, update header file");

    let globalmanager_h = fs.readFileSync(rootPath + "src/globalmanager/globalmanager.h").toString();

    let date = new Date().toString();

    globalmanager_h = globalmanager_h.replace(/\/\/\s*?apindex[\S\s]+\/\/\s*?apindex/g,
        `//apindex ${date}\r\nconst char* serverIndex = R"(${html})";\r\n//apindex`);

    fs.writeFileSync(rootPath + "src/globalmanager/globalmanager.h", globalmanager_h);

    console.log("replace html OK\n");
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