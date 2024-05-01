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

	let methodGetWaitForResponse = [];




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

	//for debug mode print messages
	//用于调试模式打印调试信息
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

	//this function used in "create.js"
	//it will accept provider execution get request
	//and route it to main hanlder
	//currently don't support return information
	//it will be added later

	//这个函数在create.js中使用
	//它用于把provider执行的http get请求
	//交由给主要处理器处理
	//目前不支持返回信息
	//之后会添加这个功能
	function fnFromGetMethod(obj, res) {
		//tid == target id
		//cpid == custom provider id
		//t == unix timestamp
		//hash == disposable key

		//tid == 目标 id
		//cpid == 自定义 provider id
		//t == unix时间戳
		//hash == 一次性密匙

		//check arguments
		//检测参数
		if (!obj.tid || !obj.cpid || !obj.t || !obj.hash) {
			return "invalid length of arguments";
		}

		//check length of every arguments
		//检查每个参数的长度
		if (!obj.tid.length || !obj.cpid.length || !obj.t.length || !obj.hash.length) {
			return "invalid arguments";

		}

		//though it will always be string arguments
		//add an extra check is more safe
		//尽管它一直是字符串
		//还是添加一个类型检测比较安全
		if ((getType(obj.tid) != "string") ||
			(getType(obj.hash) != "string")) {
			return "invalid type of arguments";
		}

		//check length of target id and hash
		//it should be 64 bytes hex string
		//检测目标id和一次性密匙的长度
		//它们应该是 64 位的 16 进制字符串
		if (obj.tid.length != 64 || obj.hash.length != 64) {
			return "invalid length of tid or pid";
		}

		let pNonHex = /[^a-fA-F0-9]/;
		let pNonNumber = /[^0-9]/;

		//check arguments if illegal
		//they should only contain 0-9 a-f A-F
		//检测参数是否合法，它们应该只包含0-9 a-f A-F
		if (pNonHex.test(obj.tid) || pNonHex.test(obj.hash)) {
			return "invalid char in tid or pid";
		}

		//check time and cpid if illegal
		//they are number
		//检测时间戳和自定义provider id是否合法
		//它们是数字
		if (pNonNumber.test(obj.t) || pNonNumber.test(obj.cpid)) {
			return "invalid char in t or pid";
		}

		//convert timestamp
		//转换时间戳
		obj.t = parseInt(obj.t);

		//check timestamp illegal or not
		//this procedure should compare to server local time
		//检测时间戳是否合法
		//这一步应该与服务器本地时间作对比

		if (obj.t < 1e6) {
			return "invalid time";
		}

		//convert custom provider id
		//转换自定义provider id
		obj.cpid = parseInt(obj.cpid);

		//id should be a uint16 type, from 0 to 65535
		//id应该是个uint16类型，从 0 到 65535
		if (obj.cpid < 0 || obj.cpid > 65535) {
			return "invalid pid";
		}

		//convert cpid into bigint
		//it will consume 8 bytes
		//but it doesn't matter it is a pid(provider id) or cpid
		//cpid转换为大整数
		//这个字段在createArrayBuffer时会占用8个字节
		//但是这无关紧要他是pid还是cpid
		obj.cpid = BigInt(obj.cpid);

		//obj.cpid = obj.cpid | 0x8000000000000000n;
		let command = 0;

		//use long command if get request require this message should be confirm
		//如果get请求指定这个消息需要确认则使用长命令
		if (obj.confirm)
			command = 0xC0000000000000BBn;
		else
			command = 0xbb;

		let fromID = getHash(new Date().getTime().toString());

		let arr = [
			//command
			//命令
			command,

			//target id
			//目标id
			obj.tid,

			//from id, generated by server
			//this id will be used to return information
			//来源 id, 由服务器生成
			//这个id会被用来返回信息
			fromID,

			//unix timestamp
			//时间戳
			obj.t,

			//disposable key
			//一次性密匙
			obj.hash.toU8a(),

			//custom provider id
			//自定义 provider id
			obj.cpid
		];


		//return provider return value 
		//返回provider的返回值

		//check value and length
		//检测值和长度
		if (obj.waitForResponse && obj.waitForResponse.length) {

			//check if legal
			//检查参数是否合法
			if (/[0-9]/.test(obj.waitForResponse)) {

				//convert to Number
				//转换为整数
				let timeout = parseInt(obj.waitForResponse);

				//check argument
				//检查参数
				if (timeout) { //NaN

					//check legitimacy
					//检查合法性
					if (timeout > 0 && timeout < 10 * 1000) {

						//create object
						//创建对象
						let json = {
							userID: fromID,
							t: new Date().getTime() + timeout,
							res: res
						};

						//add to queue
						//添加到队列
						methodGetWaitForResponse.push(json);

						return "";
					}
				}
			}
		}

		let buffer = createArrayBuffer(arr);

		webClientSendCommand(arr, buffer);

		return fromID;

	};

	//create server
	//创建服务器
	let server = createServer(blacklist, visitors, getType, print, fnFromGetMethod);

	//create websocket server
	//创建websocket服务器
	webSocket = new WebSocketServer({
		backlog: 10,
		maxPayload: 4096 * 1024,
		server: server,
		perMessageDeflate: false
	}, function () { });

	//to store firmware data and its information
	//存储固件数据和对应信息
	let customOTA = [];
	//process.customOTA = customOTA;

	webSocket.on("error", function (err) {
		console.log(err);
	});

	//find websocket client by id
	//通过id查找对应的客户端
	let findTargetByID = function (id) {
		//遍历查找
		for (let i of webSocket.clients) {
			if (i.id == id) {
				print("target client found by its id");
				return i;
			}
		}

		//didn't find target by id
		//没有找到对应id的客户端
		return {
			available: false,
			send: e => { print("coudn't find client by its id"); }
		};
	};

	/**
	 * find websocket clients by user name
	 * 通过用户名查找对应的客户端
	*/
	let findTargetByUserName = function (userName) {

		//store clients
		//存储客户端
		let arr = [];

		//traverse to find client and store it into array
		//遍历查找客户端存入数组
		for (let i of webSocket.clients) {
			if (i.userName == userName) {
				//admin user name
				//管理员账户
				arr.push(i);
			} else {
				//other users
				//其他用户
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

	/**
	 * send data to client
	 * if data has been transfered in
	 * will send data directly
	 * or, send array as data
	 * 
	 * 向客户端发送数据
	 * 如果参数传了data(ArrayBuffer)
	 * 就直接发送这个原始数据
	 * 否则会发送使用数组生成的array buffer
	*/
	let launchData = function (
		client, //client 客户端
		arr, //array 数组
		data //原始数据
	) {
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

	/**
	 * @brief
	 * when esp32 boot and connected to server
	 * it will send register information to server
	 * server will send back unix timestamp to sync time
	 * 
	 * 当esp32上电连接到服务器之后
	 * 它会把基础信息发送给服务器
	 * 服务器会把unix时间戳发回去以同步时间
	 * 
	 * @note
	 * if server enabled token authorization process
	 * esp32 will also send token authorization information
	 * 
	 * 如果服务器开启了令牌认证
	 * esp32会发送认证信息
	*/
	let register = function (client, arr) {
		if (client.unauthorizedClientTimeout) {
			//if current client just registered to server and server 
			//enabled token authorization, this client has a JS timeout timer
			//server will check information valid or not

			//如果当前客户端刚刚向服务器注册过
			//而且服务器开启了令牌认证，则当前客户端存在一个JS timeout 定时器
			//服务器会对认证信息执行检查
			if (getType(arr[1]) == "b") {
				//convet bigint to number
				//把bigint转换为number
				arr[1] = Number(arr[1]);
			}
			if (arr.length == 3 && (getType(arr[1]) == "number") && getType(arr[2]) == "u8a") {
				let time = arr[1];
				if ((new Date().getTime()) - time > 3000) {
					//because of the time just synchronized
					//so the timespan between two timestamps
					//must very short

					//因为是刚刚同步过的时间
					//所以两个时间戳的间隔必然很短
					print("used hash");
					return;
				}
				arr[2] = arr[2].toHex();
				let hash = token + arr[1];
				hash = getHash(hash);

				if (hash === arr[2]) {
					//client authorized token authorization
					//客户端通过令牌认证
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

			//this user name is administrator
			//这个用户名是管理员的
			client.userName = arr[2];

			/**
			 * @brief
			 * client reboot because of OTA update
			 * will send a message to administrator
			 * tell admin client online
			 * 
			 * 客户端因为OTA升级后重启
			 * 会发送消息给管理员
			 * 通知管理员客户端已经上线
			*/
			let clientOnline = e => {
				let indexOfOTATarget = customOTA.findIndex(e => { return e.target == client.id; });

				if (indexOfOTATarget >= 0) {
					launchData(findTargetByID(customOTA[indexOfOTATarget].admin),
						[
							0xfb,
							customOTA[indexOfOTATarget].target,
							customOTA[indexOfOTATarget].admin,
							"Client online"
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
			//发送给esp32时间戳
			launchData(client, [
				0x80,
				new Date().getTime()
			]);

			if (enableTokenAuthorize) {
				//if token authorization enabled, then send request require esp32
				//response for this process
				//如果令牌认证开启
				//就要求esp32立即回应认证
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
						//kick client if it doesn't response in time
						//如果esp32没有及时回应服务器的认证请求就踢了它
						client.terminate();
						console.log(
							"client kicked because of it didn't response token atuhorize, id: ",
							client.id);
					}, 1000);
				}, 100);
			}
		}
	};

	/**
	 * @brief
	 * esp32 response to server after server send hello
	 * esp32 回应服务器的打招呼
	*/
	let clientResponseToHello = function (client) {
		print("client response say hello, id: ", client.id);
		if (client.timeout) {
			try { clearTimeout(client.timeout); } catch (e) { }
		}
		client.t = setTimeout(e => {
			fnProactiveDetectClientOnline(client);
		}, sendHelloInterval);
	};

	/**
	 * @brief
	 * administrator send a request require esp32 update its firmware OTA
	 * 管理员发送请求要求esp32使用OTA更新固件
	*/
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

		//calc hash of firmware that admin uploaded
		//计算管理员上传的固件的数字摘要
		let hashOfFirmware = getHash(arr[3]);
		let hashFromWeb = arr[7];
		if (getType(hashFromWeb) == "u8a") {
			hashFromWeb = hashFromWeb.toHex();
		}

		if (!(hashOfFirmware === hashFromWeb)) {
			//if any data error, cancel update and push back message
			//如果数据有误，取消升级然后返回消息
			launchData(client, [
				0xfb,
				arr[1],
				arr[2],
				"Hash check failed"
			]);
			print("a web client request ota update, but hash check failed");
			return;
		}

		//firmware data valid
		//固件数据正确
		let json = {
			//esp32 id
			target: arr[1],

			//id of admin that require this esp32 update firmware
			//要求当前esp32升级固件的管理员的id
			admin: arr[2],

			//firmware
			//固件数据
			data: arr[3],

			//custom OTA data block size
			//自定义的OTA块大小
			customBlockSize: arr[6]
		};

		//find if there is same target
		//查找是否有相同的升级对象
		let index = customOTA.findIndex(e => {
			return e.target == json.target;
		});

		//add to list
		//添加到列表
		if (index >= 0) {
			customOTA[index] = json;
		} else {
			customOTA.push(json);
		}

		if (!client.id)
			client.id = arr[2];

		//send request to esp32
		//发送升级请求到esp32
		print("send websocket ota request to esp32");
		launchData(findTargetByID(json.target), [
			0xab, //websocket ota, ota升级命令
			json.admin, //web client id, 要求升级的管理员的id
			json.customBlockSize, //block size, 本次升级的OTA数据块的大小
			json.data.length, //total size, 固件总长度
			arr[4], //time, 时间戳
			arr[5] //disposable key, 一次性密匙
		]);
	};

	/**
	 * @brief
	 * esp32 request OTA data block
	 * esp32请求OTA数据块
	*/
	let esp32RequestOTABlock = function (client, arr) {
		//1 == esp32 id
		//2 == block index

		print("find target obj");
		if (getType(arr[1]) != "u8a") {
			return;
		}

		//find object
		//查找对象
		let otaTargetObj = customOTA.find(e => { return e.target == arr[1].toHex(); });

		if (!otaTargetObj) {
			print("no obj target");
			return;
		}

		//block start position
		//数据块开始位置偏移量
		let start = arr[2] * otaTargetObj.customBlockSize;

		//slice block
		//切割数据
		let block = otaTargetObj.data.slice(start, start + otaTargetObj.customBlockSize);

		//calculate block hash
		//计算数据块哈希值
		let hash = "";

		//get sha256 digest
		//获取SHA256数字摘要
		hash = new Uint8Array(getHash(block, !0));
		print("esp32 fetching block:", arr[2]);

		//send block data
		//发送OTA数据块
		launchData(client, [
			0xac, //ota data block command, ota数据块命令
			hash, //hash of single block, 当前数据块的哈希值
			block, //data, 数据块
			arr[2] //block index, 当前数据块索引
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
					100
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
						0xae, //esp32 ota progress command
						otaTargetObj.target,
						parseInt(((arr[2] * otaTargetObj.customBlockSize) / otaTargetObj.data.length) * 100)
					]);
			}
		}, 100);
	};

	/**
	 * @brief
	 * user send request require deivce send its basic information
	 * 用户发出请求要求设备发送自己的基础信息
	*/
	let webClientFindDevices = function (client, arr, data) {
		// 1 == web client id
		// 2 == user name

		if (!client) {
			return;
		}

		if (arr.length != 5) {
			return;
		}

		let target = null;

		//check this client is hacker or not
		//检测当前客户端是否合法
		target = blacklist.find(e => { return e == client.ip; });
		if (target) {
			//existed hacker will be kicked
			//非法用户给他踢了
			client.terminate();
			print("a hacker detected on blacklist");
			return;
		} else {
			//if current user had been sent same request
			//then increase its count
			//如果当前用户发出过相同请求
			//就增加他的计数
			target = administrators.find(e => { return e.ip == client.ip; });
			if (target) {
				if (target.times++ > maxFindDeviceTimes) {
					//if current user send too many request
					//kick him off
					//如果当前用户不断的发出请求
					//把他踢了然后把他ip加黑名单里
					blacklist.push(target.ip);
					client.terminate();
					print("a hacker detected");
					return;
				}
			} else {
				//if this user is new here
				//then build his document
				//如果是个新用户
				//就给他建个档案
				administrators.push({
					ip: client.ip,
					times: 0
				});
			}
		}

		//set current admin client id
		//一个用户发出查找设备请求的时候顺便把他的id记录一下
		client.id = arr[1];

		//find valid device of this user
		//查找这个用户是否拥有设备
		let targets = findTargetByUserName(arr[2]);

		if (targets.length) {
			//if user has any device
			//send broadcast to those devices
			//如果用户有设备就广播这个请求
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

	/**
	 * @brief
	 * esp32 accepted find defice request and user passed
	 * authorization, then esp32 will send its basic information
	 * 
	 * esp32接收到了寻找设备请求而且用户已通过认证
	 * 那么esp32就会发送回来它的基础信息
	*/
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
			8 response->push_back(new Element(SYSTEM_VERSION));                                  	// current structure version
			9 response->push_back(new Element(APP_VERSION));                                        // app version
			10 response->push_back(new Element(this->bufferProviders, this->bufferProvidersLength)); // providers buffer
		*/

		if (!client) {
			return;
		}

		//store nickname for debug
		//this could be removed
		//存储一下昵称方便调试信息显示
		//这个可以删除
		client.nickname = arr[6];

		//directly route original data to user
		//直接将原始数据路由到用户
		launchData(findTargetByID(arr[1]), 0, data);

	};

	/**
	 * @brief
	 * esp32 send log info to administrator
	 * esp32发送记录信息给管理员
	*/
	let esp32SendLogToWebClient = function (arr, data) {
		// 0 == 0xfb
		// 1 == board id, string
		// 2 == admin id, string
		// 3 == log, string

		//decide which method should transfer return value to source
		//决定什么方法传输返回值
		let clientFromHttpGetIndex = methodGetWaitForResponse.findIndex(e => { return e.userID == arr[2]; });
		if (clientFromHttpGetIndex < 0) {
			//websocket
			if(getType(arr[2]) == "u8a"){
				arr[2] = arr[2].toHex();
			}
			launchData(findTargetByID(arr[2]), 0, data);
		} else {
			//http

			//convert argument
			//转换参数

			let rtnValue = arr[3];
			if (!rtnValue) {
				return;
			}

			let type = getType(rtnValue);

			if (type == "number") {
				rtnValue = rtnValue.toString();
			} else if (type == "u8a") {
				rtnValue = u8a.toHex();
			} else {
				if (type != "string") {
					return;
				}
			}

			//return value from provider
			//返回provider的返回值
			methodGetWaitForResponse[clientFromHttpGetIndex].res.end(rtnValue);

			//remove object
			//移除对象
			methodGetWaitForResponse.splice(clientFromHttpGetIndex, 1);

		}
	};

	/**
	 * @brief
	 * this function handle all data sent to this server
	 * do type and security check
	 * and route it to realated part
	 * 
	 * 这个函数处理所有发送到这个服务器的信息
	 * 执行类型和安全检查
	 * 然后路由到相应部分
	*/
	function websocketIncomingMsgHandler(data, isRepeat) {

		//the data must be ArrayBuffer
		//负载数据类型必须是ArrayBuffer
		if (!getType(data) == "ab") {
			return;
		}

		//because of code design, the minmium legal data length is 2
		//因为代码设计，最小的合法数据长度是2个字节
		if (data.byteLength < 2) {
			return;
		}

		print("decode arraybuffer from client: ", this.id);

		//decode original data to an array
		//解码原始数据到一个数组
		let arr = decodeArrayBuffer(data);


		//check array
		//检查数组
		if (!arr || !arr.length) {
			return;
		}

		//the first element in array is command, it could be 1 byte or 8 bytes
		//when long command accepted(8 bytes), the type is BigInt
		//数组的第一个元素是命令，它可能是1个字节或者8个字节
		//当接收到一个长命令时，它的类型是BigInt
		let command = arr[0];

		//check command type, basic command or long command
		//检测命令类型，基础命令还是长命令
		let commandType = getType(command, !0);

		if (commandType === "b") {
			//if it's a long command, copy it
			//如果是个长命令, 拷贝一份
			let extented = command; //bigint

			//then got basic command from it
			//然后从里面取出基础命令
			command &= 0xffn;

			//then convert it to Number type for process
			//然后将其转换为Number类型方便处理
			command = Number(command);

			//if one long command had been processed and server call this function
			//the argument "isRepeat" will be set to true
			//当一个长命令已经被处理过，当前请求是服务器自己发出的
			//参数 "isRepeat" 会被设置为 true
			if (!isRepeat) {
				//current request is new one
				//当前请求是个新的请求

				//got mask byte
				//refer to /doc/extented_command_specification.txt
				//取出掩码字节
				//参考 /doc/extented_command_specification.txt
				let header = (extented & 0xff00000000000000n) >> 56n;
				header = Number(header);

				/**
				 * got id of this message
				 * id is not available when user or another send a request
				 * to require another device do something
				 * only available when deivce confirmed message
				 * 
				 * example:
				 * 
				 * A(user or a device) -> send a message need to confirm -> B(another deivce)
				 * id is zero
				 * 
				 * B -> report to server message has been confirmed -> server
				 * id is available
				 *
				 * 取出这个消息的id
				 * id在用户或另一个设备请求一个设备执行什么操作时是 不 可用的
				 * id仅在设备确认消息时才是可用的
				 * 
				 * 举例:
				 * 
				 * A(用户或者一个设备) -> 发送了一个需要确认的消息 -> B(另一个设备)
				 * 此时 id 是 0
				 * 
				 * B -> 报告服务器消息已被确认 -> 服务器
				 * 此时 id 可用
				*/
				let id = extented & 0x00ffffffff000000n;
				id = Number(id);


				if (header & 0b01000000) {
					//a client send a message shoud be confirmed
					//一个客户端发送了一个需要被确认的消息

					//currently not use
					//当前未使用
					let targetIDPosition = header & 0b00011000;

					//calculate hash of message
					//计算消息的数字摘要
					let msgHash = getHash(new Uint8Array(data).toHex() + new Date().getTime().toString(), !0);

					//got first 4 bytes from hash
					//取出数字摘要的奇案4个字节
					let msgID = (msgHash[0] << 24) | (msgHash[1] << 16) | (msgHash[2] << 8) | (msgHash[3]);

					//this number is the id of this message
					//这个数字就是这个消息的id
					msgID = Math.abs(msgID);

					//convert id to BigInt
					//把 id 转换为 BigInt 类型
					msgID = BigInt(msgID);

					//left shift it for combine it to original long command
					//左移 id 让其位置匹配对应位置
					msgID <<= 24n;

					//fill id to original long command
					//填充 id 到原始的长命令部分
					arr[0] |= msgID;

					//encode new array to ArrayBuffer
					//编码新的数组到ArrayBuffer
					//这一步其实可以直接将 id 按位或到 data 的对应部分
					//但是之前是这么写的就先这么用吧
					data = createArrayBuffer(arr);

					//create a object for confirm message
					//为确认消息创建一个对象
					let json = {
						//message id
						//消息 id
						msgID: msgID,

						//original data, for resend to target when target didn't reponse in time
						//原始数据，用来当目标未即使确认消息时重新发送
						data: data,

						//indicate this object is disposed or not
						//指示当前对象是否还有用
						disposed: false,

						//last send message time
						//最后一次发送消息的时间
						lastSendTime: new Date().getTime(),

						//indicate how many times that current message had been sent 
						//指示当前消息已经被发送多少次了
						times: 0,

						//sender or receiver
						//发送者 或 接收者
						client: this
					};

					//currently not use
					//当前未使用
					if (targetIDPosition) {
						json.targetID = arr[targetIDPosition];
					}

					//push object into array
					//对象添加到数组
					messages.push(json);
				}

				if (header & 0b00100000) {
					//target confirm message has been processed
					//目标确认消息已被处理 

					//find message object
					//找到对应的消息对象
					let msgObj = messages.find(e => { return e.msgID == id; });

					//mark it as disposed
					//将其标记为没用的
					if (msgObj) {
						msgObj.disposed = true;
					}

				}
			}
		} else {
			//the type of command only support 
			//unsigned char(uint8) and unsigned long long(uint64)
			//if it's not a BigInt, it must be uint8
			//or, consider it is illegal command

			//命令的类型仅支持单字节和 8 字节
			//它不是BigInt就一定是单字节
			//否则将其视为非法命令
			if (commandType != "u8") {
				return;
			}
		}

		//for compatibility with BigInt types, decodeArrayBuffer function didn't convert
		//8 bytes number from BigInt to Number
		//so manually convert it
		//为了兼容BigInt类型，解码时没有直接把 8 字节整数转换为Number类型
		//所以这里手动转换一下
		for (let i = 0; i < arr.length; ++i) {
			if (getType(arr[i]) == "b") {
				arr[i] = Number(arr[i]);
			}
		}

		

		//message delivery
		//消息投递
		switch (command) {
			case 0x0c:
				//client send hello
				//客户端主动打招呼
				try { this.send(createArrayBuffer([0xc0])); } catch (e) { }
				break;

			case 0xc0:
				//client send world
				//客户端回应服务器的打招呼
				clientResponseToHello(this);
				break;

			case 0x80:
				//esp32 register itself basic information after boot or reconnected
				//esp32 上电或者断线后重连向服务器注册自身信息
				register(this, arr);
				break;

			case 0xab:
				//administrator send request require esp32 start OTA update process
				//管理员发送请求要求esp32开始OTA升级
				webClientRequestOTAUpdateUsingWebsocket(this, arr);
				break;

			case 0xac:
				//esp32 request ota block
				//esp32 请求OTA升级数据块
				print("esp32 fetch block=======================================================================");
				esp32RequestOTABlock(this, arr);
				break;

			case 0xaf:
				//user send find device request
				//用户发出请求查找设备
				webClientFindDevices(this, arr, data);
				break;

			case 0xbb:
				//user or a device send request to esp32 require to execute provider
				//用户或设备发送请求到esp32要求执行provider
				webClientSendCommand(arr, data);

				break;

			case 0xfa:
				//device send back basic information
				//设备发回它的基础数据
				esp32ResponseFindDevice(this, arr, data);

				break;

			case 0xfb:
				//esp32 send log
				//esp32 发送信息
				esp32SendLogToWebClient(arr, data);
				break;

			default:
				print(arr);
		}
	};

	/**
	 * @brief
	 * server proactive detect client online or not
	 * 服务器主动探测客户端是否在线
	*/
	function fnProactiveDetectClientOnline(client) {
		//server say hello
		//服务器发出一个打招呼消息
		client.sentHello = !0;
		client.send(createArrayBuffer([0x0c]));

		//kick off client if it didn't response in time
		//如果客户端没有及时响应服务器的打招呼消息就踢了它
		client.timeout = setTimeout(() => {
			client.terminate();
			print("client didn't response server say hello, kicked");
		}, waitForClientResponseTimeout);
	};

	//callback for client in
	//有新的客户端连接到服务器时的回调函数
	webSocket.on("connection", function connection(client, req) {
		//set message type
		//设置消息类型
		client.binaryType = "arraybuffer";

		//store basic request
		//保存基础请求
		client.req = req;

		//server will send hello to client repeatedly if proactiveDetectClientOnline is enabled
		//如果服务器打开了主动探测客户端是否在线功能则会周期性的发送打招呼数据
		if (proactiveDetectClientOnline) {
			client.t = setTimeout(e => {
				fnProactiveDetectClientOnline(client);
			}, sendHelloInterval);
		}

		//get user ip address
		//获取用户的ip地址
		client.ip = /\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}/.exec(client.req.socket.remoteAddress);

		//kick it off if ip is empty
		//获取不到ip直接给它踢了
		if (!client.ip) {
			client.terminate();
			print("a hacker detected 0");
			return;
		}

		//store ip
		//保存ip
		client.ip = client.ip.toString();

		//basically this is not necessary
		//基本上这个是不需要的
		if (getType(client.ip) != "string") {
			client.terminate();
			print("a hacker detected 1");
			return;
		}

		//kick it off if ip is empty string
		//空ip直接给它踢了
		if (!client.ip.length) {
			client.terminate();
			print("a hacker detected 2");
			return;
		}

		//check this user if in blacklist
		//检测这个用户是否是黑名单上的
		let hacker = blacklist.find(e => { return e == client.ip; });
		if (hacker) {
			//kick it off if it on blacklist
			//如果在黑名单里直接给它踢了
			client.terminate();
			print("a hacker detected 3");
			return;
		}

		//clear all timer if error occured
		//如果出现出错误把所有定时器全取消了
		client.on("error", function (err) {
			try { clearTimeout(client.t); } catch (e) { }
			try { clearTimeout(client.timeout); } catch (e) { }
			try { clearTimeout(client.unauthorizedClientTimeout); } catch (e) { }
			try { clearTimeout(client.pendingTokenAuthorizeTimer); } catch (e) { }
		});

		//clear all timer if client offline
		//如果客户端掉线了把所有定时器全取消了
		client.on("close", function () {
			try { clearTimeout(client.t); } catch (e) { }
			try { clearTimeout(client.timeout); } catch (e) { }
			try { clearTimeout(client.unauthorizedClientTimeout); } catch (e) { }
			try { clearTimeout(client.pendingTokenAuthorizeTimer); } catch (e) { }
			console.log("client offline, id: ", client.id);
		});

		//handle client message
		//处理客户端的消息
		client.on("message", function (data) {
			websocketIncomingMsgHandler.apply(
				client,
				[
					//message
					//消息
					data,

					//this message is from client, so set it to false
					//这个消息时客户端发来的，所以给他设置为false
					false
				]
			);
		});
	});

	//this timer handle those messages which should be confirm but
	//target didn't response
	//这个定时器处理那些需要被确认但是目标没有及时响应的消息
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

	//this timer handle repeat broadcast and vistors requests
	//这个定时器处理重复的广播和游客请求
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
	//如果有人使用穷举法不停地发出请求，而在作出决定时保存黑名单
	//这会消耗较多的CPU和磁盘IO
	//所以使用定时器来保存黑名单
	setInterval(() => {
		fs.writeFile("./blacklist.json", JSON.stringify(blacklist), function (err) {
			if (err) {
				print("error ocuured when store blacklist");
			}
		});

		let t = new Date().getTime();


		//remove used object, those object is from http get method
		//wait for response
		//移除以前的对象，这些对象是那些从 http get 等待返回值的对象
		//但是超时未返回 返回值
		if (methodGetWaitForResponse.length) {
			for (let i = 0; i < methodGetWaitForResponse.length; ++i) {
				if (methodGetWaitForResponse[i].t < t) {
					methodGetWaitForResponse.splice(i, 1);
				}
			}
		}
	}, 10000);

	if (getType(globalConfig.port) != "number") {
		globalConfig.port = 12345;
	}

	if (globalConfig.port < 0 || globalConfig.port > 65535) {
		globalConfig.port = 12345;
	}

	//start listen
	//开启服务器监听
	server.listen(globalConfig.port);
})();