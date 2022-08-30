let fs = require("fs");

let rootPath = process.argv[process.argv.length - 1];

rootPath += "ap/";

let html = fs.readFileSync(rootPath + "apIndex.html").toString();
let js = fs.readFileSync(rootPath + "main.min.js").toString();
let ab = fs.readFileSync(rootPath + "ab.js").toString();
let hash = fs.readFileSync(rootPath + "hash.js").toString();
html = html.replace("main_js", js);
html = html.replace("ab_js", ab);
html = html.replace("hash_js", hash);
html = html.replace(/"/g, "'");
html = html.replace(/[\r\n]/g, "");
html = html.replace(/[\s]{2,}/g, " ")

fs.writeFileSync(rootPath + "out.txt", html);
//fs.writeFileSync(rootPath + "out.html", html);

console.log("ap index built");