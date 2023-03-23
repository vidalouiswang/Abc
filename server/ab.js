(function () {
	const MARK_UINT8 = 0x80;
	const MARK_UINT16 = 0x81;
	const MARK_UINT32 = 0x82;
	const MARK_UINT64 = 0x83;
	const MARK_BUFFER = 0x85;
	const MARK_STRING = 0x86;
	const MARK_EXTRA = 0x87;
	const MARK_INT8 = 0x88;
	const MARK_INT16 = 0x89;
	const MARK_INT32 = 0x90;
	const MARK_INT64 = 0x91;
	const MARK_FLOAT = 0x92;
	const MARK_DOUBLE = 0x93;

	function push1Byte(arr, n) {
		arr.push(n);
	};
	function push2Bytes(arr, n) {
		let b = new DataView(new Uint16Array([n]).buffer);
		arr.push(b.getUint8(1));
		arr.push(b.getUint8(0));
	};
	function push4Bytes(arr, n) {
		let b = new DataView(new Uint32Array([n]).buffer);
		for (let j = 3; j > -1; j--) {
			arr.push(b.getUint8(j));
		}
	};
	function push8Bytes(arr, t) {
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
	};
	function pop1Byte(arr, dv, offset, showOffset, unsigned) {
		if (showOffset) {
			arr.push({
				data: unsigned ? dv.getUint8(offset) : dv.getInt8(offset),
				offset: offset,
				length: 1
			});
		} else {
			arr.push(unsigned ? dv.getUint8(offset) : dv.getInt8(offset));
		}
		return 1;
	};
	function pop2Bytes(arr, dv, offset, showOffset, unsigned) {
		if (showOffset) {
			arr.push({
				data: unsigned ? dv.getUint16(offset) : dv.getInt16(offset),
				offset: offset,
				length: 2
			});
		} else {
			arr.push(unsigned ? dv.getUint16(offset) : dv.getInt16(offset));
		}
		return 2;
	};
	function pop4Bytes(arr, dv, offset, showOffset, unsigned) {
		if (showOffset) {
			arr.push({
				data: unsigned ? dv.getUint32(offset) : dv.getInt32(offset),
				offset: offset,
				length: 4
			});
		} else {
			arr.push(unsigned ? dv.getUint32(offset) : dv.getInt32(offset));
		}
		return 4;
	};
	function pop8Bytes(arr, dv, offset, showOffset, convertBigintToNumber) {
		let b = BigInt(0);

		let tmpU32 = new Uint32Array(2);


		for (let i = 0; i < 4; i++) {
			tmpU32[0] = tmpU32[0] + dv.getUint8(offset + i);
			if (i < 3)
				tmpU32[0] <<= 8;
		}


		for (let i = 4; i < 8; i++) {
			tmpU32[1] = tmpU32[1] + dv.getUint8(offset + i);
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
				offset: offset,
				length: 8
			});
		} else {
			arr.push(b);
		}
		return 8;
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
							push1Byte(arr, n);
						} else if (n > 255 && n < 65536) {
							arr.push(MARK_UINT16);
							push2Bytes(arr, n);
						}
						else if (n > 65535 && n < 4294967296) {
							arr.push(MARK_UINT32);
							push4Bytes(arr, n);
						} else {
							arr.push(MARK_UINT64);
							push8Bytes(arr, n);
						}
					} else {
						//signed int
						if (n >= -128) {
							arr.push(MARK_INT8);
							push1Byte(arr, n);
						} else if (n < -128 && n >= -32768) {
							arr.push(MARK_INT16);
							push2Bytes(arr, n);
						}
						else if (n < -32768 && n >= -2147483648) {
							arr.push(MARK_INT32);
							push4Bytes(arr, n);
						} else {
							arr.push(MARK_INT64);
							push8Bytes(arr, n);
						}
					}
				} else {
					//double
					//use string length to decide float or double
					let t = n.toString().length - 1;
					if (t > 5) {
						//double
						arr.push(MARK_DOUBLE);
						push8Bytes(arr, n);
					} else {
						//float
						arr.push(MARK_FLOAT);
						push4Bytes(arr, n);
					}
				}
			} else if (0 <= typeName.indexOf("bigint")) {
				let bigNumber = a[w];
				arr.push(MARK_UINT64);
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
				arr.push(MARK_STRING);

				let originalObject = new TextEncoder().encode(JSON.stringify(a[w]));

				let b = new DataView(new Uint32Array([originalObject.length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of originalObject) {
					arr.push(h);
				}
			} else if (0 <= typeName.indexOf("uint8array")) {
				arr.push(MARK_BUFFER);

				let b = new DataView(new Uint32Array([a[w].length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of a[w]) {
					arr.push(h);
				}
			} else if (0 <= typeName.indexOf("string")) {
				arr.push(MARK_STRING);

				let originalString = new TextEncoder().encode(a[w]);

				let b = new DataView(new Uint32Array([originalString.length]).buffer);
				for (let j = 3; j > -1; j--) {
					arr.push(b.getUint8(j));
				}

				for (let h of originalString) {
					arr.push(h);
				}
			} else {
				arr.push(MARK_EXTRA);

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

		let offset = 0;



		while (offset < buffer.byteLength) {
			let mark = dv.getUint8(offset);
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
					offset += pop1Byte(arr, dv, offset, showOffset, (mark == MARK_UINT8 ? true : false));
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
					offset += pop2Bytes(arr, dv, offset, showOffset, (mark == MARK_UINT16 ? true : false));
					break;

				case MARK_UINT32:
				case MARK_INT32:
					++offset;
					if (buffer.byteLength - offset < 4) {
						return returnEmpty();
					}
					offset += pop4Bytes(arr, dv, offset, showOffset, (mark == MARK_UINT32 ? true : false));
					break;
				case MARK_UINT64:
				case MARK_INT64:
					++offset;
					if (buffer.byteLength - offset < 8) {
						return returnEmpty();
					}
					offset += pop8Bytes(arr, dv, offset, showOffset, convertBigintToNumber);
					break;
				case MARK_FLOAT:
				case MARK_DOUBLE:
					++offset;
					if (buffer.byteLength - offset < (mark == MARK_FLOAT ? 4 : 8)) {
						return returnEmpty();
					}
					if (showOffset) {
						arr.push({
							data: arr.push(mark == MARK_FLOAT ? dv.getFloat32(offset) : dv.getFloat64(offset)),
							offset: offset,
							length: (mark == MARK_FLOAT ? 4 : 8)
						});
					} else {
						arr.push(mark == MARK_FLOAT ? dv.getFloat32(offset) : dv.getFloat64(offset));
					}
					offset += (mark == MARK_FLOAT ? 4 : 8);
					break;
				case MARK_BUFFER:
					++offset;

					if (buffer.byteLength - offset < 4) {
						return returnEmpty();
					}

					let len = dv.getUint32(offset);
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
					let l = dv.getUint32(offset);
					offset += 4;
					if (buffer.byteLength - offset < l) {
						return returnEmpty();
					}
					let str = new Uint8Array(l);
					for (let n = 0; n < l; n++) {
						str[n] = dv.getUint8(offset++);
					}
					str = new TextDecoder().decode(str);

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