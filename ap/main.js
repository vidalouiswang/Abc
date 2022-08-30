(e => {
    let w = window;
    let get = id => {
        return document.getElementById(id);
    };
    let showMsg = w.showMsg = (msg, timeout) => {
        timeout = timeout || 3000;
        let obj = get('msg');
        try { clearTimeout(obj.t); } catch (e) { }
        obj.innerHTML = msg;
        obj.className += ' msgShow';
        obj.t = setTimeout(() => {
            obj.className = 'msg';
        }, timeout);
    }
    let chinese = true;
    let languageContent = {
        cn: {
            ssid: "WiFi名称",
            wifiPwd: "WiFi密码",
            user: "用户名",
            userPwd: "密码",
            websocketDomain: "WebSocket主机",
            websocketPort: "WebSocket端口",
            websocketPath: "WebSocket路径",
            nickname: "昵称",
            token: "令牌",
            setArguments: "设置参数",
            reboot: "重启",
            rebootNow: "立即重启",
            rollback: "回滚固件",
            deepSleep: "睡眠10分钟",
            txtError: "错误",
            txtSet: "设置参数成功",
            txtDelayReboot: "设备将会在3秒后重启",
            txtRollback: "回滚操作成功",
            txtDeepSleep: "设备即将睡眠",
            txtConnected: "已连接",
            txtDisconnected: "已断开",
            txtInvalid: "不合法"
        },
        en: {
            ssid: "WiFi SSID",
            wifiPwd: "WiFi password",
            user: "User name",
            userPwd: "password",
            websocketDomain: "WebSocket host",
            websocketPort: "WebSocket port",
            websocketPath: "WebSocket path",
            nickname: "Nickname",
            token: "Token",
            setArguments: "Submit",
            reboot: "Reboot",
            rebootNow: "Reboot immediately",
            rollback: "Rollback firmware",
            deepSleep: "Deep sleep 10 minutes",
            txtError: "Error",
            txtSet: "Arguments set",
            txtDelayReboot: "Will reboot in 3 seconds",
            txtRollback: "Rollback OK",
            txtDeepSleep: "Will into deep sleep in 3 seconds",
            txtConnected: "Connected",
            txtDisconnected: "Disconnected",
            txtInvalid: " Invalid"
        }
    };



    (e => {
        //get language
        let language = navigator.language;
        if (language.length) {
            language = language.substring(0, 2).toLowerCase();
            if (language.startsWith("en")) {
                chinese = false;
            }
        }

        //set language
        let arr = document.getElementsByTagName('a');
        for (let i of arr) {
            if (!i.href || i.href == '') {
                i.href = 'javascript:void(0);';
            }
            i.innerText = chinese ? languageContent.cn[i.id] : languageContent.en[i.id];
        }

        arr = document.getElementsByTagName("input");
        for (let i of arr) {
            if (i.type == "text" || i.type == "password") {
                i.placeholder =
                    chinese ? languageContent.cn[i.id] :
                        languageContent.en[i.id];
            }
        }

    })();



    let connectWebsocket = w.connectWebsocket = e => {
        try {
            w.webSocket = new WebSocket('ws://' + w.location.host + ':80');
            w.webSocket.binaryType = "arraybuffer";
            w.webSocket.onerror = e => {
                showMsg((chinese ? languageContent.cn.txtError : languageContent.en.txtError) + ": " + JSON.stringify(e));
                setTimeout(() => {
                    connectWebsocket();
                }, 1000);
            };
            w.webSocket.onmessage = msg => {
                if (msg.data && msg.data.byteLength >= 2) {
                    let arr = decodeArrayBuffer(msg.data);

                    if (arr && arr.length) {
                        if (arr[0] == 0x00) {
                            //OK
                            showMsg(chinese ? languageContent.cn.txtSet : languageContent.en.txtSet);
                        } else if (arr[0] == 0x01) {
                            //reboot
                            showMsg(chinese ? languageContent.cn.txtDelayReboot : languageContent.en.txtDelayReboot);
                        } else if (arr[0] == 0x88) {
                            //rollback
                            showMsg(chinese ? languageContent.cn.txtRollback : languageContent.en.rollback);
                        } else if (arr[0] == 0x89) {
                            //deepsleep
                            showMsg(chinese ? languageContent.cn.txtDeepSleep : languageContent.en.deepSleep);
                        } else if (arr[0] == 0x90) {
                            //connect wifi
                            // if (arr[1] == 0x10) {
                            //     showMsg("无法连接到指定WiFi");
                            // } else if (arr[1] == 0x11) {
                            //     showMsg("已连接到WiFi");
                            // }
                        }
                    }
                }
            };
            w.webSocket.onopen = function () {
                showMsg(chinese ? languageContent.cn.txtConnected : languageContent.en.txtConnected);
            };

            w.webSocket.onclose = e => {
                showMsg(chinese ? languageContent.cn.txtDisconnected : languageContent.en.txtDisconnected);
                setTimeout(() => {
                    connectWebsocket();
                }, 1000);
            };
        }
        catch (e) {
            setTimeout(() => {
                connectWebsocket();
            }, 1000);
        }
    };

    w.connectWebsocket();

    let launch = w.launch = function (arr) {
        w.webSocket.send(
            createArrayBuffer(arr)
        );
    };

    get('setArguments').onclick = e => {
        let invalidArgument = argumentName => {
            showMsg(argumentName + (chinese ? languageContent.cn.txtInvalid : languageContent.en.txtInvalid));
        };

        let nickname = get('nickname').value;
        let ssid = get('ssid').value;

        if (!ssid.trim().length) {
            invalidArgument(chinese ? languageContent.cn.ssid : languageContent.en.ssid);
            return;
        }

        let wifiPwd = get('wifiPwd').value;

        if (!wifiPwd.trim().length) {
            invalidArgument(chinese ? languageContent.cn.wifiPwd : languageContent.en.wifiPwd);
            return;
        }

        let user = get('user').value;

        if (!user.trim().length) {
            invalidArgument(chinese ? languageContent.cn.user : languageContent.en.user);
            return;
        }

        let userPwd = get('userPwd').value;

        if (!userPwd.trim().length) {
            invalidArgument(chinese ? languageContent.cn.userPwd : languageContent.en.userPwd);
            return;
        }

        let domain = get('websocketDomain').value;

        if (!domain.trim().length) {
            invalidArgument(chinese ? languageContent.cn.websocketDomain : languageContent.en.websocketDomain);
            return;
        }

        let port = get('websocketPort').value;

        port = port.replace(/\s/g, "");
        port = port.length ? port : "80";
        port = parseInt(port);

        let path = get('websocketPath').value;

        path = path.replace(/\s/g, "");
        path = path.length ? path : "/";

        let token = get('token').value;

        launch(
            [
                0x00,
                nickname,
                ssid,
                wifiPwd,
                new Uint8Array(getHash(user, !0)),
                new Uint8Array(getHash(userPwd, !0)),
                domain,
                port,
                path,
                token
            ]
        );
    };

    // get('connect').onclick = function (e) {
    //     if (this.mode) {
    //         this.innerText += "!";
    //         return;
    //     }
    //     let ssid = get('ssid').value;
    //     let wifiPwd = get('wifiPwd').value;
    //     launch([
    //         0x90,
    //         ssid,
    //         wifiPwd
    //     ]);
    //     this.innerText = "连接中";
    //     this.mode = "connecting";
    // }

    get('reboot').onclick = e => {
        launch([0x01]);
    };

    get("rebootNow").onclick = e => {
        launch([0x02]);
    };

    get("rollback").onclick = e => {
        launch([0x88]);
    };

    get("deepSleep").onclick = e => {
        launch([0x89]);
    };

    (e => {
        let inputs = document.getElementsByTagName("input");
        for (let i of inputs) {
            if (i.type == "text" || i.type == "password") {
                i.onkeyup = function (e) {
                    localStorage[this.id] = this.value;
                };
            }
        }

        for (let i in localStorage) {
            let obj = get(i);
            if (obj) {
                obj.value = localStorage[i];
            }
        }

    })();
})();