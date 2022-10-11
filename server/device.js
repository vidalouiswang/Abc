(function () {
    let w = window;
    let createButton = (text, fnClick, fnDblClick, id, className) => {
        let obj = create("a");
        obj.className = className || "button";

        obj.innerText = text;
        obj.onclick = fnClick;
        obj.ondblclick = fnDblClick;
        if (id) obj.id = id;
        return obj;
    }

    class Provider {
        constructor(boardID, providerID, name, mask, isAdmin, customID) {
            this.boardID = boardID;
            this.providerID = providerID;
            if (customID !== null) {
                this.customID = customID;
                this.hasCustomID = true;
            }

            this.name = name;
            this.mask = mask;
            this.isAdmin = isAdmin;
            this.arguments = [];
            this.showToUser = false;
            this.buildContainer();

        }
        buildContainer() {
            let div = create();
            let btn = null;
            let helperID = getHash(this.boardID + this.providerID + this.name);
            div.id = helperID + "_container";
            let mask = this.mask;

            let lengthOfAArguments = mask & 0b00000111;
            this.lengthOfAArguments = lengthOfAArguments;

            let commonShowToAdmin = mask & 0b10000000;
            let questionAdmin = mask & 0b01000000;
            let adminRequired = mask & 0b00100000;
            let showToUser = mask & 0b00010000
            let encrypt = mask & 0b00001000;
            this.encrypt = encrypt ? !0 : !1;


            if (commonShowToAdmin || questionAdmin || adminRequired) {
                if (!this.isAdmin) {
                    div.style.display = "none";
                } else {
                    if (commonShowToAdmin) {
                        btn = createButton(this.name, e => { this.execute(); });
                    } else if (questionAdmin) {
                        btn = createButton(this.name, 0, e => { this.execute(); });
                    }
                }
            } else {
                div.style.display = "inline-block";
                btn = createButton(this.name, e => { this.execute(); });
                this.showToUser = true;
            }

            for (let i = 0; i < lengthOfAArguments; ++i) {
                let argument = create("input");
                argument.type = "text";
                argument.className = "LoginInput";
                argument.placeholder = i.toString();
                argument.addEventListener("keyup", e => {
                    if (/^@hex/.test(argument.value)) {
                        argument.hexMode = !0;
                        argument.placeholder = "Hex Mode (" + i.toString() + ")";
                        argument.value = "";
                    }
                    if (/^@qhex/.test(argument.value)) {
                        argument.hexMode = 0;
                        argument.placeholder = i.toString();
                        argument.value = "";
                    }
                });

                argument.addEventListener("dragover", function (e) {
                    e.preventDefault();
                    e.cancelBubble = true;
                    return false;
                });
                argument.addEventListener("drop", function (e) {
                    e.preventDefault();
                    e.cancelBubble = true;
                    if (e.dataTransfer) {
                        let file = e.dataTransfer.files[0]
                        let reader = new FileReader();
                        let fileName = file.name;

                        if (fileName.endsWith(".hex") || fileName.endsWith(".txt")) {
                            reader.readAsText(file, 'utf-8');
                        } else if (fileName.endsWith(".bin")) {
                            reader.readAsArrayBuffer(file);
                        } else {
                            showMsg("未知类型 Unknown type");
                            return;
                        }

                        reader.onload = function (e) {
                            showMsg("文件已加载 File loaded")
                            if (fileName.endsWith(".hex") || fileName.endsWith(".txt")) {
                                argument.attachedHexContent = e.target.result;
                            } else {
                                argument.attachedBinaryContent = e.target.result;
                            }
                        };
                    }

                    return false;
                });

                this.arguments.push(argument);
                div.appendChild(argument);
            }
            if (btn) {
                div.appendChild(btn);
                btn.title = "Provider ID: " + this.providerID + (this.hasCustomID ? (", Custom ID: " + this.customID) : "");
                btn.id = helperID;
            }

            if (!lengthOfAArguments) {
                div.style.display = "inline-block";
            }


            this.container = div;
        }
        execute() {
            let t = new Date().getTime();
            let hash = getHash(w.userName + w.password + t.toString(), !0);

            hash = new Uint8Array(hash);

            let arrArguments = [];
            for (let i of this.arguments) {

                if (i.hexMode || i.attachedHexContent || i.attachedBinaryContent) {
                    let u8a = null;
                    let hex = i.attachedHexContent ? i.attachedHexContent.trim() : i.value.trim();

                    if (i.attachedBinaryContent) {
                        u8a = new Uint8Array(i.attachedBinaryContent);
                    } else {
                        if (/[^0-9a-fA-F ,]/.test(hex)) {
                            showMsg("非法字符 Invalid char");
                            return;
                        }

                        if (hex.length === 2) {
                            u8a = new Uint8Array(1);
                            u8a[0] = parseInt(hex, 16);
                        } else {
                            let isSpliterExists = /[a-fA-F0-9]{1,2}( |,)[a-fA-F0-9]{1,2}/.test(hex);

                            if (isSpliterExists) {
                                let arr = hex.split(/[\s,]/);

                                if (arr.length) {
                                    for (let j = 0; j < arr.length; ++j) {
                                        arr[j] = arr[j].trim();
                                        if (!arr[j].length) {
                                            arr.splice(j, 1);
                                            if (j > 0) {
                                                j--;
                                            }
                                            continue;
                                        }
                                        arr[j] = parseInt(arr[j], 16);
                                    }
                                    u8a = new Uint8Array(arr);
                                } else {
                                    showMsg("无内容 Empty content");
                                    return;
                                }
                            } else {
                                if (hex.length % 2) {
                                    showMsg("非法长度 Invalid length");
                                    return;
                                }

                                let arr = [];

                                let unit = "";

                                for (let j = 0; j < hex.length; j += 2) {
                                    unit = hex.substring(j, j + 2).trim();
                                    if (unit.length) {
                                        arr.push(parseInt(unit, 16));
                                    }
                                }
                                u8a = new Uint8Array(arr);
                            }
                        }
                    }

                    arrArguments.push(u8a);
                } else {
                    arrArguments.push(i.value);
                }

            }

            let command = BigInt("0xC0000000000000BB"); // 0x80 | 0x40, fixed 80, 0x40 == 0b01000000, message should be confirmed

            let arr = [
                command, //command
                this.boardID, //esp32 id
                w.id, //web client id
                t, //time
                hash, //hash
                this.providerID, //provider id
            ];

            if (arrArguments.length) {
                let buffer = new Uint8Array(createArrayBuffer(arrArguments));

                if (this.encrypt) {
                    buffer = buffer.toHex();
                    buffer = encrypt(buffer, w.password, w.password.substring(0, 32), !0);
                }
                arr.push(buffer);
            }

            sendArrayBuffer(arr);
        }
    }



    class Device {
        constructor(arr) {
            this.buildManagementPanel();

            //update information
            this.updateInfo(arr);

            //make a closer id
            this.id = this.info.id;

            //container
            let deviceContainer = create();
            deviceContainer.className = "device";

            //nickname
            this.nicknameContainer = this.createNicknameContainer();

            //append elements
            deviceContainer.appendChild(this.nicknameContainer); //nickname

            this.createMoreContainer();

            //build OTA update panel
            this.buildOTAPanel();

            this.buildProviderPanel();

            document.body.appendChild(this.moreContainer);

            deviceContainer.appendChild(this.providerPanel);

            //set ref
            this.container = deviceContainer;
        }
        isAdmin() {
            return this.info.extra & 32;
        }
        createMoreContainer() {
            let div = create();
            div.className = "deviceMore";



            div.ondblclick = e => {
                let tagName = e.target.tagName;
                if (tagName.toLowerCase() == "div") {
                    this.moreContainer.className = "deviceMore";
                }
            };



            div.appendChild(createButton("id: " + this.info.id));
            div.appendChild(createButton("版本: " + this.info.firmwareVersion));
            div.appendChild(createButton("App版本: " + this.info.appVersion));

            this.moreContainer = div;
        }
        createNicknameContainer() {
            let nicknameContainer = create();

            let nickname = create("span");
            nickname.innerText = this.info.nickname;
            nicknameContainer.appendChild(nickname);

            if (this.isAdmin()) {
                this.btnMore = createButton("@", e => {
                    this.moreContainer.className += " deviceMoreShow";
                }, null, null, "smallButton");

                nicknameContainer.appendChild(this.btnMore);
            }


            return nicknameContainer;
        }
        buildManagementPanel() {
            let div = create();
            this.managePanel = div;
        }
        getNetworkFormat() {
            let mask = this.info.extra;
            if (mask == 192) {
                return "Wifi已连接,AP已开启";
            } else if (mask == 128) {
                return "Wifi已连接,AP已关闭";
            } else if (mask == 64) {
                return "Wifi未连接,AP已开启";
            }
        }
        buildProviderPanel() {
            let providerPanel = create();

            let arrProviders = decodeArrayBuffer(this.info.providers.buffer);

            this.providerPanel = providerPanel;
            let divAdminToolsWithZeroArguments = create();
            for (let i = 0; i < arrProviders.length; i++) {
                let provider = decodeArrayBuffer(arrProviders[i].buffer, !0, ["id", "mask", "name", "customID"]);
                if (!provider) {
                    continue;
                }
                let p = new Provider(
                    this.id,
                    provider.id,
                    provider.name,
                    provider.mask,
                    this.isAdmin(),
                    provider.customID !== undefined ? provider.customID : null // handle 0 custom id
                );



                if (p.container) {
                    if (p.showToUser) {
                        providerPanel.appendChild(p.container);
                    } else {

                        if (!p.lengthOfAArguments) {
                            divAdminToolsWithZeroArguments.appendChild(p.container);
                        } else {
                            this.moreContainer.appendChild(p.container);
                        }

                    }
                }

            }
            this.moreContainer.appendChild(divAdminToolsWithZeroArguments);
        }
        updateInfo(arr) {
            this.info = {
                cpu: arr[2],
                extra: arr[3],
                time: arr[4],
                heap: arr[5],
                nickname: arr[6],
                id: arr[7],
                firmwareVersion: arr[8],
                appVersion: arr[9],
                providers: arr[10]
            };
        }
        buildOTAPanel() {
            //ota
            let divOTA = create();

            this.firmware = null;

            let btnLoadFirmware = createButton("加载固件", e => {
                let btnInnerOTALoad = create("input");
                btnInnerOTALoad.type = "file";
                let reader = new FileReader();
                btnInnerOTALoad.onchange = e => {
                    let file = btnInnerOTALoad.files[0];
                    reader.readAsArrayBuffer(file);
                    reader.onload = e => {
                        this.firmware = reader.result;
                        this.showMsg("固件已加载, 大小: " + this.firmware.byteLength + "字节", 100000);
                    };
                };
                btnInnerOTALoad.click();
            });

            let btnUpdate = createButton("更新固件", e => {
                this.ota(!0);
            });

            divOTA.appendChild(btnLoadFirmware);
            divOTA.appendChild(btnUpdate);

            this.moreContainer.appendChild(divOTA);
        }
        ota() {
            //0xab
            if (!this.firmware) {
                showMsg("尚未加载固件");
                return;
            }
            showMsg("正在上传固件");

            let t = new Date().getTime();
            let hash = getHash(w.userName + w.password + t.toString(), !0);

            hash = new Uint8Array(hash);

            let u8aFirmware = new Uint8Array(this.firmware);
            let firmwareHash = new Uint8Array(getHash(u8aFirmware, !0));


            sendArrayBuffer([
                0xab,
                this.id,
                w.id,
                u8aFirmware,
                t,
                hash,
                parseInt(8 * 1024),
                firmwareHash
            ]);
        }
        otaProgress(ratio) {
            this.showMsg("升级固件: " + ratio + "%");
        }
        commandResponse(content) {
            showMsg(content);
        }
        showMsg(msg, timeout) {
            if (getType(msg) == "b") {
                msg = Number(msg);
            }
            if (getType(msg) == "number") {
                msg = msg.toString();
            }
            if (this.tShowMsg) {
                clearTimeout(this.tShowMsg);
            }
            this.nicknameContainer.children[0].innerText = msg;
            this.tShowMsg = setTimeout(() => {
                this.nicknameContainer.children[0].innerText = this.info.nickname;
            }, timeout || 10000);
        }
    }
    w.Device = Device;
})();