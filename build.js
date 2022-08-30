let child_process = require("child_process"), fs = require("fs");

(e => {

    let config = {};

    let language = {
        en: {
            input: "Input: ",
            buildingApp: " Building app: ",
            allBuilt: "All firmwares have been built",
            errorInput: "Wrong selection, input again\n",
            selectBuildApp: "Select one app to build:\n",
            selectCopyApp: "Select one app to copy:\n",
            copied: "copied\n",
            opType: "1: Build single app\n2: Build all\n3: Copy only\n4: exit\n:"
        },
        cn: {
            input: "请输入: ",
            buildingApp: "正在构建固件: ",
            allBuilt: "所有固件已构建完成",
            errorInput: "输入错误，请重新输入\n",
            selectBuildApp: "构建哪一个固件:\n",
            selectCopyApp: "拷贝哪个固件:\n",
            copied: "已拷贝\n",
            opType: "1: 单独构建\n2: 全部构建\n3: 仅拷贝\n4: 退出\n:"
        }
    };

    // build firmware
    let buildFirmware = e => {
        let stdout = child_process.execSync("export PATH=$PATH:~/.platformio/penv/bin && pio run --target clean");
        console.log(stdout.toString());
        stdout = child_process.execSync("export PATH=$PATH:~/.platformio/penv/bin && pio run");
        console.log(stdout.toString());
    };

    // create directory
    if (!fs.existsSync("./app/")) {
        fs.mkdirSync("./app/");
    }

    let apps = fs.readdirSync("./app");
    let appsRoot = "./app/";

    // load names of app
    let refresh = function () {
        apps = fs.readdirSync("./app");
        for (let i = 0; i < apps.length; i++) {
            if (!fs.statSync(appsRoot + apps[i]).isDirectory()) {
                apps.splice(i, 1);
            }
        }

        let appString = "";

        for (let i = 0; i < apps.length; i++) {
            let json = {
                index: i + 1,
                name: apps[i]
            };
            arrMete.push(json);
            appString += json.index.toString() + ": " + json.name + "\n";
        }

        appString += language[config.language].input;
        return appString;
    };

    // build all firmware
    let buildAll = function () {
        for (let i of apps) {
            if (fs.statSync(appsRoot + i).isDirectory()) {
                console.log(language[config.language].buildingApp + i);
                fs.cpSync(appsRoot + i + "/app/", "./src/app/", { recursive: true });
                buildFirmware();
            }
        }
        console.log(language[config.language].allBuilt);
    };

    let readline = require('readline');
    let rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    let query = function () {
        rl.question(language[config.language].opType, buildType);
    };

    let arrMete = [];

    let buildSingle = function (data) {
        data = parseInt(data);
        let json = arrMete.find(e => { return e.index == data; });
        if (!json) {
            query();
            return;
        }
        console.log(language[config.language].buildingApp + json.name)
        fs.cpSync(appsRoot + json.name + "/app/", "./src/app/", { recursive: true });
        buildFirmware();
        query();
    };

    let copyOnly = function (data) {
        data = parseInt(data);
        let json = arrMete.find(e => { return e.index == data; });
        if (!json) {
            query();
            return;
        }
        fs.cpSync(appsRoot + json.name + "/app/", "./src/app/", { recursive: true });
        console.log(json.name + " " + language[config.language].copied);
        query();
    };

    let buildType = function (data) {
        data = parseInt(data);
        if (data == 1) {
            let q = refresh();
            rl.question(language[config.language].selectBuildApp + q, buildSingle);
        } else if (data == 2) {
            buildAll();
            query();
        } else if (data == 3) {
            let q = refresh();
            rl.question(language[config.language].selectCopyApp + q, copyOnly);
        } else if (data == 4) {
            process.exit();
        } else {
            rl.question(language[config.language].errorInput +
                language[config.language].opType, buildType);
        }
    };
    config.language = Intl.DateTimeFormat().resolvedOptions().locale.toLowerCase();
    if (config.language == "zh-cn") {
        config.language = "cn";
    } else {
        config.language = "en";
    }

    query();

})();