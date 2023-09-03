(function () {
	const MARK_UINT8 = 1;
	const MARK_UINT16 = 2;
	const MARK_UINT32 = 4;
	const MARK_UINT64 = 8;
	const MARK_BUFFER = 10;
	const MARK_STRING = 9;
	const MARK_EXTRA = 11;
	const MARK_INT8 = -1;
	const MARK_INT16 = -2;
	const MARK_INT32 = -4;
	const MARK_INT64 = -8;
	const MARK_FLOAT = 6;
	const MARK_DOUBLE = 7;


	function pushNBytes(json) {
		for (let i = 0; i < json.byteLength; ++i) {
			json.arr.push(0);
		}
		let buffer = new Uint8Array(json.arr);
		let dv = new DataView(buffer.buffer);
		switch (json.type) {
			case "8":
				dv.setUint8(buffer.byteLength - json.byteLength, json.number);
				break;
			case "16":
				dv.setUint16(buffer.byteLength - json.byteLength, json.number, true);
				break;
			case "32":
				dv.setUint32(buffer.byteLength - json.byteLength, json.number, true);
				break;
			case "64":
				dv.setBigUint64(buffer.byteLength - json.byteLength, BigInt(json.number), true);
				break;
			case "f":
				dv.setFloat32(buffer.byteLength - json.byteLength, json.number, true);
				break;
			case "d":
				dv.setFloat64(buffer.byteLength - json.byteLength, json.number, true);
				break;
		}

		json.arr = Array.from(buffer);
		return json.arr;
	};

	function createArrayBuffer(a) {
		if (!a) return;
		if (!a.length) return;

		let oPC = Object.prototype.toString;
		let arr = [];

		for (let w = 0; w < a.length; w++) {
			typeName = oPC.call(a[w]).toLowerCase().split(" ")[1];
			if (0 <= typeName.indexOf("number")) {
				let n = a[w];
				let str = n.toString();

				if (parseInt(str) == parseFloat(str)) {
					//integer
					if (n >= 0) {
						//unsigned int
						if (n < 256) {
							arr.push(MARK_UINT8);
							arr = pushNBytes({
								arr: arr,
								byteLength: 1,
								number: n,
								type: "8"
							});
						} else if (n > 255 && n < 65536) {
							arr.push(MARK_UINT16);
							arr = pushNBytes({
								arr: arr,
								byteLength: 2,
								number: n,
								type: "16"
							});
						}
						else if (n > 65535 && n < 4294967296) {
							arr.push(MARK_UINT32);
							arr = pushNBytes({
								arr: arr,
								byteLength: 4,
								number: n,
								type: "32"
							});
						} else {
							arr.push(MARK_UINT64);
							arr = pushNBytes({
								arr: arr,
								byteLength: 8,
								number: n,
								type: "64"
							});
						}
					} else {
						//signed int
						if (n >= -128) {
							arr.push(MARK_INT8);
							arr = pushNBytes({
								arr: arr,
								byteLength: 1,
								number: n,
								type: "8"
							});
						} else if (n < -128 && n >= -32768) {
							arr.push(MARK_INT16);
							arr = pushNBytes({
								arr: arr,
								byteLength: 2,
								number: n,
								type: "16"
							});
						}
						else if (n < -32768 && n >= -2147483648) {
							arr.push(MARK_INT32);
							arr = pushNBytes({
								arr: arr,
								byteLength: 4,
								number: n,
								type: "32"
							});
						} else {
							arr.push(MARK_INT64);
							arr = pushNBytes({
								arr: arr,
								byteLength: 8,
								number: n,
								type: "64"
							});
						}
					}
				} else {
					//double
					//use string length to decide float or double
					let t = n.toString().length - 1;
					if (t > 5) {
						//double
						arr.push(MARK_DOUBLE);
						arr = pushNBytes({
							arr: arr,
							byteLength: 8,
							number: n,
							type: "d"
						});
					} else {
						//float
						arr.push(MARK_FLOAT);
						arr = pushNBytes({
							arr: arr,
							byteLength: 4,
							number: n,
							type: "f"
						});
					}
				}
			} else if (0 <= typeName.indexOf("bigint")) {
				arr.push(MARK_UINT64);
				arr = pushNBytes({
					arr: arr,
					byteLength: 8,
					number: a[w],
					type: "64"
				});
			} else if (0 <= typeName.indexOf("uint8array")) {
				arr.push(MARK_BUFFER);

				arr = pushNBytes({
					arr: arr,
					byteLength: 4,
					number: a[w].length,
					type: "32"
				});

				for (let h of a[w]) {
					arr.push(h);
				}
			} else if (0 <= typeName.indexOf("string")) {
				arr.push(MARK_STRING);

				let originalString = new TextEncoder().encode(a[w]);

				arr = pushNBytes({
					arr: arr,
					byteLength: 4,
					number: originalString.length + 1,
					type: "32"
				});

				for (let h of originalString) {
					arr.push(h);
				}
				arr.push(0);
			} else {

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

		let offset = 0;

		while (offset < buffer.byteLength) {
			let mark = dv.getUint8(offset);
			if (mark > 247) {
				mark = mark - 256;
			}
			if (arr.length && decodeFirstByte) {
				break;
			}
			switch (mark) {
				case MARK_UINT8:
				case MARK_INT8:
					++offset;
					if (buffer.byteLength - offset < 1) {
						return returnEmpty();
					}

					if (showOffset) {
						arr.push({
							data: mark == MARK_UINT8 ? dv.getUint8(offset) : dv.getInt8(offset),
							offset: offset,
							length: 1
						});
					} else {
						arr.push(mark == MARK_UINT8 ? dv.getUint8(offset) : dv.getInt8(offset));
					}
					++offset;

					if (decodeFirstByte) {
						break;
					}
					break;
				case MARK_UINT16:
				case MARK_INT16:
					++offset;
					if (buffer.byteLength - offset < 2) {
						return returnEmpty();
					}
					if (showOffset) {
						arr.push({
							data: mark == MARK_UINT16 ? dv.getUint16(offset, true) : dv.getInt16(offset, true),
							offset: offset,
							length: 2
						});
					} else {
						arr.push(mark == MARK_UINT16 ? dv.getUint16(offset, true) : dv.getInt16(offset, true));
					}
					offset += 2;
					break;

				case MARK_UINT32:
				case MARK_INT32:
					++offset;
					if (buffer.byteLength - offset < 4) {
						return returnEmpty();
					}
					if (showOffset) {
						arr.push({
							data: mark == MARK_UINT32 ? dv.getUint32(offset, true) : dv.getInt32(offset, true),
							offset: offset,
							length: 4
						});
					} else {
						arr.push(mark == MARK_UINT32 ? dv.getUint32(offset, true) : dv.getInt32(offset, true));
					}
					offset += 4;
					break;
				case MARK_UINT64:
				case MARK_INT64:
					++offset;
					if (buffer.byteLength - offset < 8) {
						return returnEmpty();
					}
					if (showOffset) {
						arr.push({
							data: mark == MARK_UINT64 ? dv.getBigUint64(offset, true) : dv.getBigInt64(offset, true),
							offset: offset,
							length: 8
						});
					} else {
						arr.push(mark == MARK_UINT64 ? dv.getBigUint64(offset, true) : dv.getBigInt64(offset, true));
					}
					offset += 8;
					break;
				case MARK_FLOAT:
				case MARK_DOUBLE:
					++offset;
					if (buffer.byteLength - offset < (mark == MARK_FLOAT ? 4 : 8)) {
						return returnEmpty();
					}
					if (showOffset) {
						arr.push({
							data: arr.push(mark == MARK_FLOAT ? dv.getFloat32(offset, true) : dv.getFloat64(offset, true)),
							offset: offset,
							length: (mark == MARK_FLOAT ? 4 : 8)
						});
					} else {
						arr.push(mark == MARK_FLOAT ? dv.getFloat32(offset, true) : dv.getFloat64(offset, true));
					}
					offset += (mark == MARK_FLOAT ? 4 : 8);
					break;
				case MARK_BUFFER:
					++offset;

					if (buffer.byteLength - offset < 4) {
						return returnEmpty();
					}

					let len = dv.getUint32(offset, true);
					offset += 4;

					if (buffer.byteLength - offset < len) {
						return returnEmpty();
					}

					if (showOffset) {
						if (len + offset > buffer.byteLength) {
							arr.push({
								offset: offset,
								length: len
							});
							break;
						}
					}
					let u8a = new Uint8Array(len);
					for (let n = 0; n < len; n++) {
						u8a[n] = dv.getUint8(offset++);
					}
					if (showOffset) {
						arr.push({
							data: u8a,
							offset: offset - len - 1,
							length: len
						});
					} else {
						arr.push(u8a);
					}
					break;
				case MARK_STRING:
					++offset;
					if (buffer.byteLength - offset < 4) {
						return returnEmpty();
					}
					let l = dv.getUint32(offset, true) - 1;
					offset += 4;
					if (buffer.byteLength - offset < l) {
						return returnEmpty();
					}
					let str = new Uint8Array(l);
					for (let n = 0; n < l; n++) {
						str[n] = dv.getUint8(offset++);
					}
					str = new TextDecoder().decode(str);
					++offset;

					if (showOffset) {
						arr.push({
							data: str,
							offset: offset - l - 1,
							length: l
						});
					} else {
						arr.push(str);
					}
					break;
				default:
					return;
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
