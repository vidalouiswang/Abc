(function () {
    let w = window;
    w.rememberPassword = false;
    let U8A = Uint8Array;
    w.devices = [];

    let get = w.get = id => {
        return document.getElementById(id);
    };

    let create = w.create = (tag, inlineBlock) => {
        tag = tag || "div";
        let obj = document.createElement(tag);
        if (inlineBlock) {
            obj.style.display = "inline-block";
        }
        return obj;
    };

    w.getType = function getType(target, moreInfo) {
        let type = Object.prototype.toString.call(target).toLowerCase().split(" ")[1];
        type = type.substring(0, type.length - 1);
        if (type == "array") {
            return "a";
        } else if (type == "uint8array") {
            return "u8a";
        } else if (type == "string") {
            return type;
        } else if (type == "number") {
            if (moreInfo) {
                if (target < 0) {
                    return type;
                } else if (target >= 0 && target < 0xff) {
                    return "u8";
                } else if (target >= 0xff && target < 0xffff) {
                    return "u16";
                } else if (target >= 0xffff && target < 0xffffffff) {
                    return "u32";
                } else {
                    return "u64";
                }
            } else {
                return type;
            }
        } else if (type == "bigint") {
            return "b";
        } else if (type == "object") {
            if (moreInfo) {
                if (target.constructor.name) {
                    return target.constructor.name;
                } else {
                    return "uo"; //unknown object
                }
            } else {
                return "o";
            }
        } else if (type == "arraybuffer") {
            return "ab";
        } else if (type == "function") {
            return "f";
        } else if (type == "date") {
            return "d";
        } else if (type == "dataview") {
            return "dv";
        } else {
            return type;
        }
    };

    U8A.prototype.toHex = function () {
        let e = "";
        for (let i of this) {
            let t;
            if (!i) {
                t = "00";
            } else {
                t = i.toString(16);
                if (t.length == 1) {
                    t = "0" + t;
                }
            }
            e += t;
        }
        return e;
    };

    String.prototype.toU8a = function () {
        if (this.length % 2) return;
        let hex = this;
        let u8a = new U8A(hex.length / 2);
        let k = 0;
        for (let i = 0; i < hex.length; i += 2) {
            u8a[k++] = parseInt(hex.substring(i, i + 2), 16);
        }
        return u8a;
    };

    let decrypt = w.decrypt = (plain, key, iv, outputUint8Array) => {
        if (!plain) return "";
        key = CryptoJS.enc.Hex.parse(key);
        iv = CryptoJS.enc.Hex.parse(iv);
        plain = CryptoJS.enc.Hex.parse(plain);
        plain = CryptoJS.enc.Base64.stringify(plain);
        let decrypt = CryptoJS.AES.decrypt(plain, key, { iv: iv, mode: CryptoJS.mode.CBC, padding: CryptoJS.pad.Pkcs7 });
        let decryptedStr = decrypt.toString(CryptoJS.enc.Hex);
        if (outputUint8Array) {
            return decryptedStr.toU8a();
        } else {
            return decryptedStr;
        }
    };

    w.encrypt = (plain, key, iv, outputUint8Array) => {
        if (!plain) return "";
        key = CryptoJS.enc.Hex.parse(key);
        iv = CryptoJS.enc.Hex.parse(iv);
        let srcs = CryptoJS.enc.Hex.parse(plain);
        let encrypted = CryptoJS.AES.encrypt(srcs, key, { iv: iv, mode: CryptoJS.mode.CBC, padding: CryptoJS.pad.Pkcs7 });
        let output = encrypted.ciphertext.toString();
        if (!outputUint8Array) {
            return output;
        }
        else {
            return output.toU8a();
        }
    };

    (e => {
        let ls = localStorage;
        let J = JSON;
        class LSAsArray {
            constructor(key) {
                this.key = key;
                this.arr = 0;
                this.exists = false;
                if (ls[key] && ls[key].length) {
                    try {
                        this.arr = J.parse(ls[key]);
                        this.exists = !0;
                    } catch (e) {
                        this.arr = [];
                    }
                } else {
                    this.arr = [];
                }
            }
            update() {
                ls[this.key] = J.stringify(this.arr);
            }
            find(fn) {
                return this.arr.find(fn);
            }
            size() {
                return this.arr.length;
            }
            filter(fn) {
                return this.arr.filter(fn);
            }
            findIndex(fn) {
                return this.arr.findIndex(fn);
            }
            push(data, dontUpdate) {
                this.arr.push(data);
                if (!dontUpdate) {
                    this.update();
                }
            }
            splice(index, length, dontUpdate) {
                this.arr.splice(index, (length || 1));
                if (!dontUpdate) {
                    this.update();
                }
            }
            spliceIfExists(fn, dontUpdate) {
                let index = this.findIndex(fn);
                if (index >= 0) {
                    this.splice(index, dontUpdate);
                }
            }
            pushIfNotExists(fn, data, dontUpdate) {
                this.set(fn, data, dontUpdate);
            }
            get(fn, returnJson) {
                let b = this;
                let index = b.findIndex(fn);
                if (index >= 0) {
                    if (returnJson) {
                        return {
                            index: index,
                            data: b.arr[index]
                        };
                    } else {
                        return b.arr[index];
                    }
                }
            }
            set(fn, data, dontUpdate) {
                let b = this;
                let index = b.findIndex(fn);
                if (index >= 0) {
                    b.arr[index] = data;
                } else {
                    b.arr.push(data);
                }
                if (!dontUpdate) {
                    b.update();
                }
            }
            concat(data, dontUpdate) {
                let b = this;
                if (data instanceof LSAsArray) {
                    b.arr = b.arr.concat(data.arr);
                } else {
                    b.arr = b.arr.concat(data);
                }
                if (!dontUpdate) {
                    b.update;
                }
            }
            forEach(fn) {
                for (let i of this.arr) {
                    fn(i);
                }
            }
        };
        w.LSAsArray = LSAsArray;
    })();

    let users = new LSAsArray("users")

    w.setEventEmitter = prototypeOrObject => {
        prototypeOrObject.on = (event, fn, id, obj, disposable) => {
            if (!prototypeOrObject["@" + event]) {
                prototypeOrObject["@" + event] = [];
            }

            if (!id) {
                prototypeOrObject["@" + event].push({
                    id: id,
                    fn: fn,
                    obj: obj,
                    disposable: disposable
                });
                return;
            }

            let target = prototypeOrObject["@" + event].find(e => {
                return e.id == id;
            });

            if (!target) {
                prototypeOrObject["@" + event].push({
                    id: id,
                    fn: fn,
                    obj: obj,
                    disposable: disposable
                });
            }
        };

        prototypeOrObject.emit = (event, data) => {
            let arr = prototypeOrObject["@" + event];
            if (!arr) {
                return;
            }

            for (let i = 0; i < arr.length; i++) {
                if (arr[i].obj) {
                    arr[i].run = !arr[i].fn.apply(arr[i].obj, [data]);
                } else {
                    arr[i].run = !arr[i].fn(data);
                }
            }
            let len = arr.length;
            for (let i = 0; i < len; i++) {
                if (arr[i]) {
                    if (arr[i].run && arr[i].disposable) {
                        arr.splice(i, 1);
                        i--;
                    }
                }

            }
        };
    };

    let showMsg = w.showMsg = (msg, timeout) => {
        let msgObj = get("divMsg");
        if (msgObj.t) { try { clearTimeout(msgObj.t); } catch (e) { } }
        msgObj.className = "divMsg divMsgShow";
        msgObj.innerHTML = msg;
        msgObj.t = setTimeout(() => {
            msgObj.className = "divMsg";
        }, timeout || 3000);
    };

    (e => {
        setEventEmitter(w);

        w.on("connected", e => {
            w.connected = true;
            showMsg("已建立连接");
        });

        w.on("unknownData", e => {
            showMsg("接收到了未知消息");
        });

        w.on("disconnected", e => {
            w.connected = false;
            showMsg("已断开连接");
        });

        w.on("connectionError", e => {
            w.connected = false;
            showMsg("连接发生错误");
        });

        w.on("unknownCommand", function (data) {
            showMsg("接收到了未知命令" + data.toString());
        });

        w.on("newDevice", function (arr) {
            let device = w.devices.find(e => { return e.id == arr[7]; });
            if (!device) {
                device = new Device(arr);
                w.devices.push(device);
                get("devices").appendChild(device.container);
            } else {
                device.updateInfo(arr);
            }
        });

        let obj = get("btnRemPwd");
        let dot = get("remPwdDot");
        obj.onclick = e => {
            if (!w.rememberPassword) {
                w.rememberPassword = true;
                dot.className += " remPwdDotSelected";
            } else {
                w.rememberPassword = false;
                dot.className += "remPwdDot";
                if (w.readPwdFromCache) {
                    showMsg("是否清除保存的密码?" +
                        "<a class='smallButton' onclick='removeUserInfo()'>是</a>",
                        10000)
                }
            }
        };
    })();


    w.removeUserInfo = e => {
        delete localStorage.users;
        showMsg("密码已清除");
        get("remPwdDot").className = "remPwdDot";
    };

    w.generateHash = e => {
        let t = new Date().getTime().toString();
        return {
            t: t,
            hash: getHash(w.userName + w.password + t)
        };
    };

    let findDevice = id => {
        return w.devices.find(e => { return e.id == id; });
    }

    let msgHandler = msg => {
        msg = msg.data;
        if (getType(msg) != "ab") {
            w.emit("unknownData");
        }

        let arr = decodeArrayBuffer(msg);

        if (arr && arr.length) {
            let command = arr[0];

            let commandType = getType(command, !0);


            if (commandType === "b") {
                let offset = BigInt(0xff);
                command &= offset;
                command = Number(command);
            } else {
                if (commandType != "u8") {
                    return;
                }
            }

            switch (command) {
                case 0x0c: //hello world
                    {
                        sendArrayBuffer([0xc0]);
                        break;
                    }
                case 0xfa: //esp32 send back basic information
                    {
                        w.emit("newDevice", arr);
                        break;
                    }
                case 0xae: //esp32 ota progress
                    {
                        let device = findDevice(arr[1]);
                        if (device)
                            device.otaProgress(arr[2]);
                        break;
                    }
                case 0xb6: //esp32 provider callback response
                    {
                        let device = findDevice(arr[2]);
                        if (device) device.commandResponse(arr[3]);
                        break;
                    }
                case 0xfb: //esp32 log
                    {
                        arr[1] = getType(arr[1]) == "u8a" ? arr[1].toHex() : arr[1];
                        let device = findDevice(arr[1]);
                        if (device)
                            if (arr.length == 5) {
                                let decryptedBuffer = decrypt(arr[3].toHex(), w.password, w.password.substring(0, 32), !0);
                                if (decryptedBuffer && decryptedBuffer.length) {
                                    device.showMsg(decodeArrayBuffer(decryptedBuffer.buffer))
                                }
                            } else {
                                device.showMsg(
                                    getType(arr[3]) == "u8a" ?
                                        decodeArrayBuffer(arr[3].buffer) :
                                        arr[3]
                                );
                            }

                        break;
                    }
                default:
                    w.emit("unknownCommand", command);
            }
        }
    };

    w.resetWebsocket = function () {
        w.webSocket = new WebSocket("ws://" + w.location.host);
        webSocket.binaryType = "arraybuffer";
        webSocket.onopen = function () {
            w.emit("connected");
        };
        webSocket.onclose = function () {
            w.emit("disconnected");
            setTimeout(() => {
                w.resetWebsocket();
            }, 1000);
        };
        webSocket.onmessage = function (msg) {
            msgHandler(msg);
        };
        webSocket.onerror = function (e) {
            w.emit("connectionError");
        };
    };

    w.resetWebsocket();

    w.sendArrayBuffer = arr => {
        webSocket.send(createArrayBuffer(arr));
    };

    let login = (userName, password, isPasswordHashed) => {
        w.password = password = isPasswordHashed ? password : getHash(password);

        get("divLogin").className += " divLoginHide";
        get("devices").className += " devicesShow";

        if (w.rememberPassword) {
            if (userName.length && password.length) {
                users.set(e => { return e.userName == userName; }, {
                    userName: userName,
                    password: password
                });
                showMsg("密码已存储");
            }

        }

        w.userName = userName = getHash(userName);

        w.id = getHash(userName + password + "admin");

        let t = new Date().getTime();
        let hash = new Uint8Array(getHash(userName + password + t.toString(), !0));

        let command = BigInt("0xC0000000000000AF"); // 0x80 | 0x40, fixed 80, 0x40 == 0b01000000, message should be confirmed

        sendArrayBuffer([
            command,
            w.id,
            w.userName,
            t,
            hash
        ]);
    };

    (e => {
        if (users.size()) {
            if (users.size() == 1) {
                w.on("connected", e => {
                    w.readPwdFromCache = true;
                    w.rememberPassword = true;
                    get("remPwdDot").className += " remPwdDotSelected";

                    setTimeout(() => {
                        let user = users.get(e => { return !0; });
                        if (w.rememberPassword)
                            login(user.userName, user.password, !0);
                    }, 1000);
                });
            } else {
                users.forEach(user => {
                    let btn = create("a");
                    btn.className = "button";
                    btn.innerText = user.userName;
                    btn.onclick = function () {
                        w.readPwdFromCache = true;
                        login(user.userName, user.password, !0);
                    };

                    get("btnContainer").appendChild(btn);
                });
            }
        }
    })();



    get("login").onclick = function () {
        login(get("userName").value, get("password").value, 0);
    };


    (e => {
        let arr = document.getElementsByTagName("a");
        for (let i of arr) {
            if (i.className.indexOf("button") >= 0 && !i.href) {
                i.href = "javascript:void(0);";
            }
        }
    })();
})()