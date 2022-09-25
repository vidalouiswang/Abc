(e => {
    let fs = require('fs');

    let lan = "cn";

    let config = {
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

    let language = {
        en: {
            op: "1. Add OTA update device\n2. Remove OTA update device\n3. Enable device OTA update\n4. Disable device OTA update\n5. Add/update server settings\n6. Add/update administrator\n7. Exit\nInput:",
            add: "Input target device info, format:\nID(required) ^ app name(required) ^ OTA update block size(bytes, optional, default is 8192)\nInput use \"^\" to separate, separator allow to have space around with\n:",
            info: "\nInput information\nID: %id\napp name: %appName\nOTA update block size: %ota\nData added\n",
            error: "Illegal input, please input again\n",
            whichOne: "Which one?\n:",
            empty: "No device\n",
            removeSuccessful: "Remove successful\n",
            success: "Success",
            allDeviceEnabled: "All deivce enabled\n",
            allDeviceDisabled: "All device disabled\n",
            setServerInfo: "Input server connection information\nformat:\n(domain or IP):port/path(path is optional)\n",
            setUserName: "User name:\n",
            setPassword: "Password:\n"
        },
        cn: {
            op: "1. 添加自动OTA升级设备\n2. 移除自动OTA升级设备\n3. 打开设备OTA升级\n4. 关闭设备OTA升级\n5. 添加/修改服务器设置\n6. 添加/修改管理员\n7. 退出\n请输入:",
            add: "请输入目标设备信息, 格式为:\nID(必填) ^ App名称(必填) ^ OTA升级块大小(以字节计, 可选, 默认8192字节)\n数据以\"^\"分隔, 分隔符周围可以包含空格\n:",
            info: "\n输入的信息为\nID: %id\n应用名称: %appName\nOTA升级块大小: %ota\n数据已添加\n",
            error: "输入有误, 请重新输入\n",
            whichOne: "哪个设备?\n:",
            empty: "无自动OTA升级设备\n",
            removeSuccessful: "移除成功\n",
            success: "成功",
            allDeviceEnabled: "没有需要打开OTA升级的设备\n",
            allDeviceDisabled: "没有需要关闭OTA升级的设备\n",
            setServerInfo: "请输入服务器信息\n格式为:\n(域名或IP):端口/路径(路径可选)\n",
            setUserName: "设置用户名:\n",
            setPassword: "设置密码:\n"
        }
    };

    let updateConfig = function () {
        fs.writeFileSync("./autoOTAConfig.json", JSON.stringify(config));
    };

    if (fs.existsSync("./autoOTAConfig.json")) {
        let c = null;
        try {
            c = JSON.parse(fs.readFileSync("./autoOTAConfig.json").toString());
        } catch (e) { c = null; }
        if (c) {
            config = c;
        }
    } else {
        updateConfig();
    }

    lan = Intl.DateTimeFormat().resolvedOptions().locale.toLowerCase();
    if (lan == "zh-cn") {
        lan = "cn";
    } else {
        lan = "en";
    }

    let readline = require('readline');
    let rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });
    let query;

    let add = function () {
        rl.question(language[lan].add, function (info) {
            if (!info.length) {
                query();
                return;
            }

            info = /\s*?(?<id>[a-fA-F0-9]{64})\s*?(\^\s*?(?<appName>[\S]+)?\s*?(\^\s*?(?<ota>[0-9]+)?)?)?/.exec(info);
            if (!info || !info.groups) {
                console.log(language[lan].error);
                query();
                return;
            }
            let template = language[lan].info;
            template = template.replace("%id", info.groups.id);
            template = template.replace("%appName", info.groups.appName);
            template = template.replace("%ota", info.groups.ota || "8192");
            console.log(template);

            let targetIndex = config.table.findIndex(e => { return e.id == info.groups.id; });

            let json = {
                id: info.groups.id,
                appName: info.groups.appName || info.groups.id,
                blockSize: info.groups.ota ? parseInt(info.groups.ota) : 8192,
                enable: true
            };

            if (targetIndex >= 0) {
                config.table[targetIndex] = json;
            } else {
                config.table.push(
                    json
                );
            }

            updateConfig();
            query();
        });
    };

    let remove = function () {
        if (!config.table.length) {
            console.log(language[lan].empty);
            query();
            return;
        }
        console.log();

        for (let i = 0; i < config.table.length; ++i) {
            console.log("" + (i + 1) + ": " + config.table[i].appName)
        }

        rl.question(language[lan].whichOne, function (index) {
            if (!index.length) {
                query();
                return;
            }

            index = parseInt(index);
            index--;

            if (index >= config.table.length) {
                console.log(language[lan].error);
                query();
                return;
            }


            config.table.splice(index, 1);
            updateConfig();
            console.log(language[lan].removeSuccessful);

            query();

        });
    };

    let toggleOTA = function (enable) {
        let arr = config.table.filter(e => { return enable ? !e.enable : e.enable; })
        if (!arr.length) {
            if (enable) {
                console.log(language[lan].allDeviceEnabled);
            } else {
                console.log(language[lan].allDeviceDisabled);
            }

            query();
            return;
        }
        let k = 0;
        for (let i of arr) {
            i.index = k;
            console.log("" + (i.index + 1) + ": " + i.appName);
        }
        rl.question(language[lan].whichOne, function (index) {
            if (!index.length) {
                query();
                return;
            }
            index--;

            let targetIndex = config.table.findIndex(e => { return e.id == arr[index].id; });

            if (targetIndex >= 0) {
                config.table[targetIndex].enable = enable;
            }
            updateConfig();
            console.log(language[lan].success);
            query();
        });
    };

    let updateServerSettings = function () {
        rl.question(language[lan].setServerInfo, function (serverInfo) {
            if (!serverInfo.length) {
                console.log(language[lan].error);
                query();
                return;
            }

            serverInfo = /(?<domain>[\S]+)(:|：)(?<port>\d+)(\/(?<path>[\S]+))?/.exec(serverInfo);

            if (serverInfo) {
                config.domain = serverInfo.groups.domain;
                config.port = serverInfo.groups.port ? parseInt(serverInfo.groups.port) : 80;
                config.path = serverInfo.groups.path || "/";

                console.log(serverInfo.groups.domain,
                    serverInfo.groups.port || "80",
                    serverInfo.groups.path || "/"
                );

                updateConfig();
                console.log(language[lan].success);
            } else {
                console.log(language[lan].error);
            }
            query();

        });
    };

    let updateAdmin = function () {
        rl.question(language[lan].setUserName, function (userName) {
            config.adminUserName = userName;
            rl.question(language[lan].setPassword, function (password) {
                config.adminPassword = password;
                updateConfig();
                console.log(language[lan].success);
                query();
            });
        });
    };

    let select = function (content) {
        if (!content.length) {
            query();
            return;
        }
        content = parseInt(content);
        switch (content) {
            case 1:
                add();
                break;
            case 2:
                remove();
                break;
            case 3:
                toggleOTA(true);
                break;
            case 4:
                toggleOTA(false);
                break;
            case 5:
                updateServerSettings();
                break;
            case 6:
                updateAdmin();
                break;
            case 7:
                process.exit(0);

            default:
                console.log(language[lan].error);
                query();
        }
    };

    query = function () {
        rl.question(language[lan].op, select);
    };

    query();

})();