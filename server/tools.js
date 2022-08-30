let fs = require('fs');
let lastUpdateTime = 0;

function refresh() {
    let html = fs.readFileSync("./source/index.html").toString();
    let hash = fs.readFileSync("./hash.min.js").toString();
    let aes = fs.readFileSync("./aes.min.js").toString();
    let ab = fs.readFileSync("./ab.min.js").toString();
    let device = fs.readFileSync("./device.min.js").toString();
    let index = fs.readFileSync("./index.min.js").toString();


    html = html.replace(/\/\/hash\.min\.js\/\//g, hash);
    html = html.replace(/\/\/aes\.min\.js\/\//g, aes);
    html = html.replace(/\/\/ab\.min\.js\/\//g, ab);
    html = html.replace(/\/\/index\.min\.js\/\//g, index);
    html = html.replace(/\/\/device\.min\.js\/\//g, device);


    html = html.replace(/(\r|\n){1,}/g, "");
    html = html.replace(/\s{2,}/g, " ");

    if (!fs.existsSync("./pro/")) {
        fs.mkdirSync("./pro/");
    }

    fs.writeFileSync("./pro/index.html", html);
    fs.writeFileSync("./index.html", html);
    console.log("success");
    lastUpdateTime = new Date().getTime();
};

fs.watch("./", function (event, filename) {
    if (new Date().getTime() - lastUpdateTime > 1000)
        refresh();
});
