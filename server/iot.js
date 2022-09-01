(function () {

	// build default config
	// 建立默认配置
	let globalConfig = {
		//server port 服务器端口
		port: 12345,

		//interval to detect client if online 
		//检测客户端是否在线的时间间隔
		sendHelloInterval: 10 * 1000,

		//if client didn't response in time after detection packet sent
		//it will be kick off by server
		//如果客户端没有在预定时间内响应服务器
		//则它会被服务器踢出
		waitForClientResponseTimeout: 3 * 1000,

		//server will detect client online status or not
		//服务器是否会主动探测客户端是否在线
		proactiveDetectClientOnline: true,

		//a(n) administrator or user(real of fake) will send request to server
		//reqire server send broadcast to those clients belong to this user
		//this valve is a limitation of times that one client could send this request
		//if a client send too many find deivce request in a short time
		//it will be consider as a hacker
		//一个管理员或者用户向服务器发出查找客户端请求时
		//服务器会向对应的那些客户端转发请求
		//这个阈值决定了一个客户端最多能发送多少次查找设备请求
		//如果一个客户端在短时间内大量发送请求
		//则这个客户端被认为是非法客户端
		maxFindDeviceTimes: 100,

		//a key that a client could use this server to transfer data
		//if "token" has been set, and "enableTokenAuthorize" set to true
		//then your device must set same token
		//or it will be kicked off
		//一个决定设备是否可以使用这个服务器中转数据的密匙
		//如果这个字段被定义而且 enableTokenAuthorize 被设置为true
		//那么你的设备也必须设置同样的token
		//否则会被服务器踢掉

		//attention: this key should be set at "globalConfig.json"
		//注意: 这个值最好在 "globalConfig.json"中设置
		token: "",

		//enable token authorization or not
		//是否启用token认证
		enableTokenAuthorize: false
	};

	let fs = require("fs");
	let WebSocketServer = require("ws").WebSocketServer;

	//encode and decode arraybuffer
	let { createArrayBuffer, decodeArrayBuffer } = require("./ab");

	//SHA256
	let getHash = require("./hash");

	//basic server
	let createServer = require("./create");

	//load config
	if (fs.existsSync("./globalConfig.json")) {
		let json = null;
		try { json = JSON.parse(fs.readFileSync("./globalConfig.json").toString()); } catch (e) { json = null; }
		if (json) {
			globalConfig = json;
		} else {
			//will cover old config file if the data is invalid
			//如果配置文件数据有问题那么会使用默认配置覆盖它
			fs.writeFileSync("./globalConfig.json", JSON.stringify(globalConfig));
		}
	} else {
		//will generate one if config does not exists
		//如果配置文件不存在就生成一个
		fs.writeFileSync("./globalConfig.json", JSON.stringify(globalConfig));
	}

	//debug mode will print more information for debug works
	//调试模式会打印更多调试信息
	let debugMode = false;

	let enableTokenAuthorize = globalConfig.enableTokenAuthorize ? true : false;

	let token = globalConfig.token || "";

	let proactiveDetectClientOnline = globalConfig.proactiveDetectClientOnline ? true : false;

	let sendHelloInterval = globalConfig.sendHelloInterval || (10 * 1000);

	let waitForClientResponseTimeout = globalConfig.waitForClientResponseTimeout || (3 * 1000);

	let maxFindDeviceTimes = globalConfig.maxFindDeviceTimes || 100;

	//this array stored those messages that need to confirm
	//这个数组存储那些需要被确认的消息
	let messages = [];

	//this array stored potential administrators
	//这个数组存储可能的管理员
	let administrators = [];

	//this array stored visitors information
	//这个数组存储访问者信息
	let visitors = [];

	//黑名单
	let blacklist = [];




	Uint8Array.prototype.toHex = function () {
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
		let u8a = new Uint8Array(hex.length / 2);
		let k = 0;
		for (let i = 0; i < hex.length; i += 2) {
			u8a[k++] = parseInt(hex.substring(i, i + 2), 16);
		}
		return u8a;
	};

	//for type detection
	//用于检测数据类型
	function getType(target, moreInfo) {
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


	//load blacklist
	//加载黑名单
	if (fs.existsSync("./blacklist.json")) {
		let arr = null;

		try {
			arr = JSON.parse(fs.readFileSync("./blacklist.json").toString());
		} catch (e) {
			arr = null;
		}
		if (getType(arr) == "a") {
			blacklist = arr;
		}
	}





	//websocket server instance
	//websocket 服务器实例
	let webSocket = null;

	let print = function () {
		if (debugMode) {
			if (arguments.callee) {
				if (arguments.callee.caller) {
					console.log("function name: [", (arguments.callee.caller.name + " ]" || " empty ]"));
				}
			}

			console.log.apply(null, arguments);
			console.log("==========");
		}
	};

	function fnFromGetMethod(obj) {

		if (!obj.tid || !obj.cpid || !obj.t || !obj.hash) {
			//console.log(obj);
			return "invalid length of arguments";
		}

		if (!obj.tid.length || !obj.cpid.length || !obj.t.length || !obj.hash.length) {
			return "invalid arguments";

		}

		if ((getType(obj.tid) != "string") ||
			(getType(obj.hash) != "string")) {
			return "invalid type of arguments";
		}

		if (obj.tid.length != 64 || obj.hash.length != 64) {
			return "invalid length of tid or pid";
		}

		let pNonHex = /[^a-fA-F0-9]/;
		let pNonNumber = /[^0-9]/;

		if (pNonHex.test(obj.tid) || pNonHex.test(obj.hash)) {
			return "invalid char in tid or pid";
		}

		if (pNonNumber.test(obj.t) || pNonNumber.test(obj.cpid)) {
			return "invalid char in t or pid";
		}

		obj.t = parseInt(obj.t);

		if (obj.t < 1e6) {
			return "invalid time";
		}

		obj.cpid = parseInt(obj.cpid);

		if (obj.cpid < 0 || obj.cpid > 65535) {
			return "invalid pid";
		}


		obj.cpid = BigInt(obj.cpid);
		//obj.cpid = obj.cpid | 0x8000000000000000n;
		let command = 0;
		if (obj.confirm)
			command = 0xC0000000000000BBn;
		else
			command = 0xbb;

		let fromID = getHash(new Date().getTime().toString());

		let arr = [
			command,
			obj.tid,
			fromID,
			obj.t,
			obj.hash.toU8a(),
			obj.cpid
		];

		let buffer = createArrayBuffer(arr);

		webClientSendCommand(arr, buffer);

		return fromID;

	};

	//create server
	let server = createServer(blacklist, visitors, getType, print, fnFromGetMethod);

	//bind server to websocket
	webSocket = new WebSocketServer({
		backlog: 10,
		maxPayload: 1000 * 1024,
		server: server,
		perMessageDeflate: false
	}, function () {

	});

	let customOTA = [];
	process.customOTA = customOTA;

	webSocket.on("error", function (err) {
		console.log(err);
	});

	//find websocket client by id
	let findTargetByID = function (id) {
		for (let i of webSocket.clients) {
			if (i.id == id) {
				print("target client found by its id");
				return i;
			}
		}
		return {
			available: false,
			send: e => { print("coudn't find client by its id"); }
		};
	};

	//find websocket clients by user name
	let findTargetByUserName = function (userName) {
		let arr = [];
		for (let i of webSocket.clients) {
			if (i.userName == userName) {
				arr.push(i);
			} else {
				if (i.users) {
					for (let j of i.users) {
						if (userName == j) {
							arr.push(i);
						}
					}
				}
			}

		}
		return arr;
	};

	//send data to client
	let launchData = function (client, arr, data) {
		if (!client) {
			print("no client");
			return;
		}
		if (!arr && !data) {
			print("no data");
			return;
		}
		try {
			if (data) {
				client.send(data);
				print("array buffer sent");
			} else {
				client.send(createArrayBuffer(
					arr
				));
				print("array sent, id: ", client.id, "\ncontent: \n", arr);
			}
		} catch (e) {
			print("send data error");
			print(e);
			return false;
		}
		return true;
	};

	//esp32 online
	//send current timestamp to esp32
	let register = function (client, arr) {
		//esp32 register or response authorizing request

		if (client.unauthorizedClientTimeout) {
			if (getType(arr[1]) == "b") {
				arr[1] = Number(arr[1]);
			}
			if (arr.length == 3 && (getType(arr[1]) == "number") && getType(arr[2]) == "u8a") {
				let time = arr[1];
				if ((new Date().getTime()) - time > 3000) {
					console.log("used hash");
					return;
				}
				arr[2] = arr[2].toHex();
				let hash = token + arr[1];
				hash = getHash(hash);

				if (hash === arr[2]) {
					try { clearTimeout(client.unauthorizedClientTimeout); } catch (e) { }
					print("client authorized token process");
				}
			}
		} else {
			//0 == command
			//1 == esp32 id
			//2 == user name(admin)
			//3 == 0x01, new firmware boot || 0x00 none
			//4 == users buffer || 0x00

			if (getType(arr[1]) == "u8a") {
				arr[1] = arr[1].toHex();
			}
			if (getType(arr[2]) == "u8a") {
				arr[2] = arr[2].toHex();
			}

			//esp32 id
			client.id = arr[1];

			//admin
			client.userName = arr[2];

			let clientOnline = e => {
				let indexOfOTATarget = customOTA.findIndex(e => { return e.target == client.id; });

				if (indexOfOTATarget >= 0) {
					launchData(findTargetByID(customOTA[indexOfOTATarget].admin),
						[
							0xfb,
							customOTA[indexOfOTATarget].target,
							customOTA[indexOfOTATarget].admin,
							"客户端已上线"
						]);
					customOTA.splice(indexOfOTATarget, 1);
				}
			};

			if (getType(arr[3]) == 0x01) {
				clientOnline();
			}

			if (getType(arr[4]) == "u8a") {
				client.users = decodeArrayBuffer(arr[4].buffer);
				for (let i = 0; i < client.users.length; i++) {
					client.users[i] = client.users[i].toHex();
				}
			}
			//send time to client
			launchData(client, [
				0x80,
				new Date().getTime()
			]);

			if (enableTokenAuthorize) {
				client.pendingTokenAuthorizeTimer = setTimeout(() => {
					//send token authorizing request
					let x = parseInt(Math.random() * 0xff);
					let y = parseInt(Math.random() * 0xff);
					launchData(client, [
						0x80,
						x,
						y
					]);

					client.unauthorizedClientTimeout = setTimeout(() => {
						// kick client if it doesn't response in time
						client.terminate();
						console.log(
							"client kicked because of it didn't response token atuhorize, id: ",
							client.id);
					}, 1000);
				}, 100);
			}
		}
	};

	//esp32 response to server after server send hello
	let clientResponseToHello = function (client) {
		print("client response say hello, id: ", client.id);
		if (client.timeout) {
			try { clearTimeout(client.timeout); } catch (e) { }
		}
		client.t = setTimeout(e => {
			fnProactiveDetectClientOnline(client);
		}, sendHelloInterval);
	};

	//web client pushed a request update firmware using websocket
	let webClientRequestOTAUpdateUsingWebsocket = function (client, arr) {

		/*
			0 0xab,
			1 this.id,
			2 w.id,
			3 new Uint8Array(this.firmware),
			4 t,
			5 hash
			6 block size
			7 hash of firmware
		*/

		if (arr.length != 8) {
			return;
		}

		// calc hash
		let hashOfFirmware = getHash(arr[3]);
		let hashFromWeb = arr[7];
		if (getType(hashFromWeb) == "u8a") {
			hashFromWeb = hashFromWeb.toHex();
		}

		if (!(hashOfFirmware === hashFromWeb)) {
			launchData(client, [
				0xfb,
				arr[1],
				arr[2],
				"Hash check failed"
			]);
			print("a web client request ota update, but hash check failed");
			return;
		}

		let json = {
			target: arr[1],
			admin: arr[2],
			data: arr[3],
			customBlockSize: arr[6]
		};

		let index = customOTA.findIndex(e => {
			return e.target == json.target;
		});

		//add to list
		if (index >= 0) {
			customOTA[index] = json;
		} else {
			customOTA.push(json);
		}

		//send request
		print("send websocket ota request to esp32");
		launchData(findTargetByID(json.target), [
			0xab, //websocket ota
			json.admin, //web client id
			json.customBlockSize, //block size
			json.data.length, //total size
			arr[4], //time
			arr[5] //hash
		]);
	};

	//esp32 request ota data block
	let esp32RequestOTABlock = function (client, arr) {
		//1 == esp32 id
		//2 == block index

		print("find target obj");
		let otaTargetObj = customOTA.find(e => { return e.target == arr[1].toHex(); });

		if (!otaTargetObj) {
			print("no obj target");
			return;
		}

		//block start position
		let start = arr[2] * otaTargetObj.customBlockSize;

		//slice block
		let block = otaTargetObj.data.slice(start, start + otaTargetObj.customBlockSize);

		//calculate block hash
		let hash = "";

		//get sha256 digest
		hash = new Uint8Array(getHash(block, !0));
		print("esp32 fetching block:", arr[2]);

		//send block data
		launchData(client, [
			0xac, //ota data block command
			hash, //hash of single block
			block, //data
			arr[2] //block index
		]);

		// 0 == 0xfb
		// 1 == board id, string
		// 2 == web client id, string
		// 3 == log, string
		if (!block.length) {
			launchData(findTargetByID(otaTargetObj.admin),
				[
					0xfb,
					otaTargetObj.target,
					otaTargetObj.admin,
					"OTA升级完成"
				]);
			otaTargetObj.data = null;
		}

		/*
			1 == esp32 id
			2 == progress
		*/

		setTimeout(() => {
			if (otaTargetObj.data) {
				launchData(findTargetByID(otaTargetObj.admin),
					[
						0xae,
						otaTargetObj.target,
						parseInt(((arr[2] * otaTargetObj.customBlockSize) / otaTargetObj.data.length) * 100)
					]);
			}
		}, 500);
	};

	//web client search esp32
	let webClientFindDevices = function (client, arr, data) {
		// 1 == web client id
		// 2 == user name

		let target = null;

		target = blacklist.find(e => { return e == client.ip; });
		if (target) {
			client.terminate();
			print("a hacker detected on blacklist");
			return;
		} else {
			target = administrators.find(e => { return e.ip == client.ip; });
			if (target) {
				if (target.times++ > maxFindDeviceTimes) {
					blacklist.push(target.ip);
					client.terminate();
					print("a hacker detected");
					return;
				}
			} else {
				administrators.push({
					ip: client.ip,
					times: 0
				});
			}
		}



		if (!client) {
			return;
		}

		if (arr.length != 5) {
			return;
		}

		//set current web client id
		client.id = arr[1];

		let targets = findTargetByUserName(arr[2]);


		if (targets.length) {
			for (let i of targets) {
				launchData(i, 0, data);
			}
		}
	};

	//web client send a command to esp32
	let webClientSendCommand = function (arr, data) {
		//web client send command to esp32
		/*
			1 == esp32 id
			2 == web client id
			3 == time
			4 == hash
			5 == command(for esp32 execute)
		*/
		print("web client send command to esp32: ", arr[5]);

		launchData(findTargetByID(arr[1]), 0, data);
	};

	//esp32 send basic information after web client request matched devices
	let esp32ResponseFindDevice = function (client, arr, data) {
		/*
			0 response->push_back(new Element(CMD_FIND_DEVICE_RESPONSE));                           // response to find device 0xaf
			1 response->push_back(new Element(adminID));                                            // web client id
			2 response->push_back(new Element((uint16_t)(ESP.getCpuFreqMHz())));                    // cpu freq
			3 response->push_back(new Element(extraInfo));                                          // extra info
			4 response->push_back(new Element(globalTime->getTime()));                              // current timestamp
			5 response->push_back(new Element((uint32_t)(ESP.getFreeHeap())));                      // free heap
			6 response->push_back(new Element(this->nickname->getString().c_str()));                // nickname of this board
			7 response->push_back(new Element(this->UniversalID->getHex().c_str()));                // id of this board
			8 response->push_back(new Element(SYSTEM_VERSION));                                  // current structure version
			9 response->push_back(new Element(APP_VERSION));                                        // app version
			10 response->push_back(new Element(this->bufferProviders, this->bufferProvidersLength)); // providers buffer
		*/
		//console.log("device response find device");
		//console.log(arr);
		if (!client) {
			return;
		}

		client.nickname = arr[6];

		launchData(findTargetByID(arr[1]), 0, data);

	};

	//esp32 send log to web client
	let esp32SendLogToWebClient = function (arr, data) {
		// 0 == 0xfb
		// 1 == board id, string
		// 2 == web client id, string
		// 3 == log, string

		launchData(findTargetByID(arr[2]), 0, data);
	};

	function websocketIncomingMsgHandler(data, isRepeat) {
		if (!getType(data) == "ab") {
			return;
		}

		if (data.byteLength < 2) {
			return;
		}

		print("decode arraybuffer from client: ", this.id);

		let arr = decodeArrayBuffer(data);

		if (!arr || !arr.length) {
			return;
		}

		let command = arr[0];
		let commandType = getType(command, !0);

		if (commandType === "b") {
			let extented = command; //bigint
			command &= 0xffn;
			command = Number(command);
			if (!isRepeat) {
				let header = (extented & 0xff00000000000000n) >> 56n;
				header = Number(header);
				let id = extented & 0x00ffffffff000000n;
				id = Number(id);


				if (header & 0b01000000) {
					// a client send a message shoud be confirmed
					let targetIDPosition = header & 0b00011000;
					let msgHash = getHash(new Uint8Array(data).toHex() + new Date().getTime().toString(), !0);
					let msgID = (msgHash[0] << 24) | (msgHash[1] << 16) | (msgHash[2] << 8) | (msgHash[3]);
					msgID = Math.abs(msgID);

					msgID = BigInt(msgID);
					msgID <<= 24n;

					arr[0] |= msgID;

					data = createArrayBuffer(arr);
					let json = {
						msgID: msgID,
						data: data,
						disposed: false,
						lastSendTime: new Date().getTime(),
						times: 0,
						client: this
					};

					if (targetIDPosition) {
						json.targetID = arr[targetIDPosition];
					}
					messages.push(json);
				}

				if (header & 0b00100000) {
					// target confirm message has been processed
					let msgObj = messages.find(e => { return e.msgID == id; });
					if (msgObj) {
						msgObj.disposed = true;
					}

				}
			}
		} else {
			if (commandType != "u8") {
				return;
			}
		}

		for (let i = 0; i < arr.length; ++i) {
			if (getType(arr[i]) == "b") {
				arr[i] = Number(arr[i]);
			}
		}

		switch (command) {
			case 0x0c:
				//client send hello
				try { this.send(createArrayBuffer([0xc0])); } catch (e) { }
				break;

			case 0xc0:
				//client send world
				clientResponseToHello(this);
				break;

			case 0x80:
				//esp32 register
				register(this, arr);
				break;

			case 0xab:
				//web client send ota request
				webClientRequestOTAUpdateUsingWebsocket(this, arr);
				break;

			case 0xac:
				//esp32 request ota block
				print("esp32 fetch block=======================================================================");
				esp32RequestOTABlock(this, arr);
				break;

			case 0xaf:
				//find device
				webClientFindDevices(this, arr, data);
				break;

			case 0xbb:
				//web client send command to esp32
				webClientSendCommand(arr, data);

				break;

			case 0xfa:
				//device send back basic information
				esp32ResponseFindDevice(this, arr, data);

				break;

			case 0xfb:
				//esp32 send log
				esp32SendLogToWebClient(arr, data);
				break;

			default:
				print(arr);
		}
	};

	function fnProactiveDetectClientOnline(client) {
		//server say hello
		client.sentHello = !0;
		client.send(createArrayBuffer([0x0c]));

		//kick off client if it didn't response in time
		client.timeout = setTimeout(() => {
			client.terminate();
			print("client didn't response server say hello, kicked");
		}, waitForClientResponseTimeout);
	};

	//callback for client in
	webSocket.on("connection", function connection(client, req) {
		//set message type
		client.binaryType = "arraybuffer";

		client.req = req;

		//check if client online
		if (proactiveDetectClientOnline) {
			client.t = setTimeout(e => {
				fnProactiveDetectClientOnline(client);
			}, sendHelloInterval);
		}

		client.ip = /\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}/.exec(client.req.socket.remoteAddress);

		if (!client.ip) {
			client.terminate();
			print("a hacker detected 0");
			return;
		}

		client.ip = client.ip.toString();

		if (getType(client.ip) != "string") {
			client.terminate();
			print("a hacker detected 1");
			return;
		}

		if (!client.ip.length) {
			client.terminate();
			print("a hacker detected 2");
			return;
		}

		let hacker = blacklist.find(e => { return e == client.ip; });
		if (hacker) {
			client.terminate();
			print("a hacker detected 3");
			return;
		}


		client.on("error", function (err) {
			try { clearTimeout(client.t); } catch (e) { }
			try { clearTimeout(client.timeout); } catch (e) { }
			try { clearTimeout(client.unauthorizedClientTimeout); } catch (e) { }
			try { clearTimeout(client.pendingTokenAuthorizeTimer); } catch (e) { }
		});

		client.on("close", function () {
			try { clearTimeout(client.t); } catch (e) { }
			try { clearTimeout(client.timeout); } catch (e) { }
			try { clearTimeout(client.unauthorizedClientTimeout); } catch (e) { }
			try { clearTimeout(client.pendingTokenAuthorizeTimer); } catch (e) { }
			console.log("client offline, id: ", client.id);
		});

		client.on("message", function (data) {
			websocketIncomingMsgHandler.apply(client, [data, false]);
		});
	});

	setInterval(() => {
		if (messages.length) {
			let t = new Date().getTime();
			for (let i of messages) {
				if (t - i.lastSendTime > 1000 && !i.disposed) {
					websocketIncomingMsgHandler.apply(i.client, [i.data, true]);
					if (++i.times > 10) {
						i.disposed = true;
					}
				}
			}
			for (let i = 0; i < messages.length; ++i) {
				if (i.disposed) {
					messages.splice(i, 1);
					--i;
				}
			}
		}
	}, 1000);

	setInterval(() => {
		for (let i = 0; i < administrators.length; ++i) {
			if (--administrators[i].times <= 0) {
				administrators.splice(i, 1);
			}
		}
		for (let i = 0; i < visitors.length; ++i) {
			if (--visitors[i].times <= 0) {
				visitors.splice(i, 1);
			}
		}
	}, 10000);

	//if someone attck your server and store blacklist in related function
	//it may use much cpu of server
	//so store blacklist using timer
	setInterval(() => {
		fs.writeFile("./blacklist.json", JSON.stringify(blacklist), function (err) {
			if (err) {
				print("error ocuured when store blacklist");
			}
		});
	}, 10000);

	server.listen(globalConfig.port || 12345);
})();