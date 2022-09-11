(function () {
	function createArrayBuffer(a) {
		if (!a) return;
		if (!a.length) return;

		let oPC = Object.prototype.toString;
		let arr = [];

		for (let w = 0; w < a.length; w++) {
			typeName = oPC.call(a[w]).toLowerCase().split(" ")[1];
			if (0 <= typeName.indexOf("number")) {
				if (a[w] < 256) {
					arr.push(0x80);

					arr.push(a[w]);
				} else if (a[w] > 255 && a[w] < 65536) {
					arr.push(0x81);

					let b = new DataView(new Uint16Array([a[w]]).buffer);
					arr.push(b.getUint8(1));
					arr.push(b.getUint8(0));
				}
				else if (a[w] > 65535 && a[w] < 4294967296) {
					arr.push(0x82);

					let b = new DataView(new Uint32Array([a[w]]).buffer);
					for (let j = 3; j > -1; j--) {
						arr.push(b.getUint8(j));
					}
				} else if (a[w] >= 4294967296) {
					arr.push(0x83);

					let t = a[w];
					let high = new DataView(new Uint32Array([t & 0xffffffff]).buffer);
					t = parseInt(t / 0xffffffff);
					let low = new DataView(new Uint32Array([t & 0xffffffff]).buffer);
					let bigNumber = [];

					for (let i = 0; i < 4; i++) {
						bigNumber.push(high.getUint8(i));
					}
					for (let i = 0; i < 4; i++) {
						bigNumber.push(low.getUint8(i));
					}

					bigNumber.reverse();

					for (let i of bigNumber) {
						arr.push(i);
					}

				}
			} else if (0 <= typeName.indexOf("bigint")) {
				let bigNumber = a[w];
				arr.push(0x83);
				let t = BigInt(0xff); //minify doesn't support 0xffn, so use BigInt
				let offset = BigInt(8);
				let tmpArr = [];
				for (let i = 0; i < 8; ++i) {
					tmpArr.push(Number(bigNumber & t));
					bigNumber >>= offset;
				}
				tmpArr.reverse();

				for (let i of tmpArr) {
					arr.push(i);
				}
			} else if (0 <= typeName.indexOf("object")) {
				arr.push(0x84);

				let originalObject = new TextEncoder().encode(JSON.stringify(a[w]));

				let b = new DataView(new Uint32Array([originalObject.length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of originalObject) {
					arr.push(h);
				}
			} else if (0 <= typeName.indexOf("uint8array")) {
				arr.push(0x85);

				let b = new DataView(new Uint32Array([a[w].length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of a[w]) {
					arr.push(h);
				}
			} else if (0 <= typeName.indexOf("string")) {
				arr.push(0x86);

				let originalString = new TextEncoder().encode(a[w]);

				let b = new DataView(new Uint32Array([originalString.length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of originalString) {
					arr.push(h);
				}
			} else {
				arr.push(0x87);

				let originalUnknown = new TextEncoder().encode(JSON.stringify(a[w]));

				

				let b = new DataView(new Uint32Array([originalUnknown.length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of originalUnknown) {
					arr.push(h);
				}
			}
		}

		return new Uint8Array(arr).buffer;

	};

	function decodeArrayBuffer(buffer, convertBigintToNumber, names, decodeFirstByte, showOffset) {
		if (!buffer) return;
		if (!buffer.byteLength) return;
		let arr = [];
		let dv;

		let returnEmpty = e => {
			if (names) {
				return {};
			} else {
				return [];
			}
		};

		try {
			dv = new DataView(buffer);
		} catch (e) {
			//console.log("error when decode: ");
			//console.log(e);
			return returnEmpty();
		}

		for (let h = 0; h < buffer.byteLength;) {
			if (!(dv.getUint8(h) ^ 0x80)) { //1byte
				h++;
				if (buffer.byteLength - h < 1) {
					return returnEmpty();
				}
				if (showOffset) {
					arr.push({
						data: dv.getUint8(h),
						offset: h,
						length: 1
					});
					h++;
				} else {
					arr.push(dv.getUint8(h++));
				}

				if (decodeFirstByte) {
					break;
				}

			} else if (!(dv.getUint8(h) ^ 0x81)) { //2bytes
				h++;
				if (buffer.byteLength - h < 2) {
					return returnEmpty();
				}
				if (showOffset) {
					arr.push({
						data: dv.getUint16(h),
						offset: h,
						length: 2
					});
				} else {
					arr.push(dv.getUint16(h));
				}
				h += 2;

			} else if (!(dv.getUint8(h) ^ 0x82)) { //4bytes
				h++;
				if (buffer.byteLength - h < 4) {
					return returnEmpty();
				}
				if (showOffset) {
					arr.push({
						data: arr.push(dv.getUint32(h)),
						offset: h,
						length: 4
					});
				} else {
					arr.push(dv.getUint32(h));
				}
				h += 4;

			} else if (!(dv.getUint8(h) ^ 0x83)) { //8bytes bigint
				h++;

				if (buffer.byteLength - h < 8) {
					return returnEmpty();
				}

				let b = BigInt(0);

				let tmpU32 = new Uint32Array(2);


				for (let i = 0; i < 4; i++) {
					tmpU32[0] = tmpU32[0] + dv.getUint8(h + i);
					if (i < 3)
						tmpU32[0] <<= 8;
				}


				for (let i = 4; i < 8; i++) {
					tmpU32[1] = tmpU32[1] + dv.getUint8(h + i);
					if (i < 7)
						tmpU32[1] <<= 8;
				}

				b += BigInt(tmpU32[0]);
				b <<= BigInt(32);
				b += BigInt(tmpU32[1]);

				if (convertBigintToNumber) {
					b = Number(b);
				}

				if (showOffset) {
					arr.push({
						data: b,
						offset: h,
						length: 8
					});
				} else {
					arr.push(b);
				}
				h += 8;
			} else if (!(dv.getUint8(h) ^ 0x84)) { //object
				h++;

				if (buffer.byteLength - h < 4) {
					return returnEmpty();
				}

				let len = dv.getUint32(h);
				h += 4;

				if (buffer.byteLength - h < len) {
					return returnEmpty();
				}

				let j = { offset: h };
				let obj = new Uint8Array(len);
				for (let n = 0; n < len; n++) {
					obj[n] = dv.getUint8(h++);
				}
				obj = new TextDecoder().decode(obj);
				try {
					obj = JSON.parse(obj);
					if (showOffset) {
						j.data = obj;
						j.length = len;
						arr.push(j);
					} else {
						arr.push(obj);
					}
				} catch (e) { arr.push(obj); }

			} else if (!(dv.getUint8(h) ^ 0x85)) { //u8a
				h++;

				if (buffer.byteLength - h < 4) {
					return returnEmpty();
				}

				let len = dv.getUint32(h);
				h += 4;

				if (buffer.byteLength - h < len) {
					return returnEmpty();
				}

				if (showOffset) {
					if (len + h > buffer.byteLength) {
						arr.push({
							offset: h,
							length: len
						});
						break;
					}
				}
				let u8a = new Uint8Array(len);
				for (let n = 0; n < len; n++) {
					u8a[n] = dv.getUint8(h++);
				}
				if (showOffset) {
					arr.push({
						data: u8a,
						offset: h - len - 1,
						length: len
					});
				} else {
					arr.push(u8a);
				}
			} else if (!(dv.getUint8(h) ^ 0x86)) { //string
				h++;
				if (buffer.byteLength - h < 4) {
					return returnEmpty();
				}
				let len = dv.getUint32(h);
				h += 4;
				if (buffer.byteLength - h < len) {
					return returnEmpty();
				}
				let str = new Uint8Array(len);
				for (let n = 0; n < len; n++) {
					str[n] = dv.getUint8(h++);
				}
				str = new TextDecoder().decode(str);
				arr.push(str);
			} else if (!(dv.getUint8(h) ^ 0x87)) { //object
				h++;
				if (buffer.byteLength - h < 4) {
					return returnEmpty();
				}
				let len = dv.getUint32(h);
				h += 4;
				if (buffer.byteLength - h < len) {
					return returnEmpty();
				}
				let obj = new Uint8Array(len);
				for (let n = 0; n < len; n++) {
					obj[n] = dv.getUint8(h++);
				}
				obj = new TextDecoder().decode(obj);
				try {
					obj = JSON.parse(obj);
					arr.push(obj);
				} catch (e) {
					arr.push(obj);
				}
			} else {
				return arr;
			}
		}
		if (!names) {
			return arr;
		}
		if (!names.length) {
			return arr;
		}
		let obj = {};
		for (let w = 0; w < names.length; w++) {
			if (w > arr.length - 1) {
				obj[names[w]] = null;
			} else {
				obj[names[w]] = arr[w];
			}
		}
		return obj;
	};

	if (typeof window == "object") {
		window.createArrayBuffer = createArrayBuffer;
		window.decodeArrayBuffer = decodeArrayBuffer;
	} else {
		module.exports = {
			createArrayBuffer: createArrayBuffer,
			decodeArrayBuffer: decodeArrayBuffer
		};
	}
})();