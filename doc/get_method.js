/**
 * @brief
 * 
 * You could use GET method through http(s) to execute provider
 * 你可以使用http(s)的GET方法执行provider
 * 
 * The following codes is a template, you could follow this template
 * use other languages to code.
 * 
 * 以下代码为示例代码，你可以根据这个模板用其他语言编程。
*/

/**
 * @brief
 * 
 * The following part is software SHA256 function, this is optional.
 * 下面SHA256数字摘要功能，可以去掉。
*/

// modify content from here===========
// 修改以下内容

let userName = "your_user_name_你的用户名";
let password = "password_密码";

let domain = "your_host_domain_or_ip_address_你的域名或者ip地址";

// port of your server
// 你的服务器端口
let port = 80;

// esp32 id, hex string
// esp32 id, 16进制字符串
let targetID = "98fc3c00a31fcdf285c3be2a98a09f80b161b97c53b8591036ce7a38d5003ea8";

// custom provider id
// 自定义的provider id
let cpid = "0";

(function () {
    let getHash = null;

    (function () {
        let rotateRight = (n, x) => { return (x >>> n) | (x << (32 - n)) }

        let choice = (x, y, z) => { return (x & y) ^ (~x & z) }

        let majority = (x, y, z) => { return (x & y) ^ (x & z) ^ (y & z) }

        let sha256_Sigma0 = x => { return rotateRight(2, x) ^ rotateRight(13, x) ^ rotateRight(22, x) }

        let sha256_Sigma1 = x => { return rotateRight(6, x) ^ rotateRight(11, x) ^ rotateRight(25, x) }

        let sha256_sigma0 = x => { return rotateRight(7, x) ^ rotateRight(18, x) ^ (x >>> 3) }

        let sha256_sigma1 = x => { return rotateRight(17, x) ^ rotateRight(19, x) ^ (x >>> 10) }

        let sha256_expand = (W, j) => { return (W[j & 0x0f] += sha256_sigma1(W[(j + 14) & 0x0f]) + W[(j + 9) & 0x0f] + sha256_sigma0(W[(j + 1) & 0x0f])) }
        let K256 = new Array(0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2);
        let ihash, count, buffer;
        let sha256_hex_digits = "0123456789abcdef";

        let safe_add = (x, y) => { let lsw = (x & 0xffff) + (y & 0xffff); let msw = (x >> 16) + (y >> 16) + (lsw >> 16); return (msw << 16) | (lsw & 0xffff) }

        let sha256_init = () => {
            ihash = new Array(8);
            count = new Array(2);
            buffer = new Array(64);
            count[0] = count[1] = 0;
            ihash[0] = 0x6a09e667;
            ihash[1] = 0xbb67ae85;
            ihash[2] = 0x3c6ef372;
            ihash[3] = 0xa54ff53a;
            ihash[4] = 0x510e527f;
            ihash[5] = 0x9b05688c;
            ihash[6] = 0x1f83d9ab;
            ihash[7] = 0x5be0cd19;
        }

        let sha256_transform = () => {
            let a, b, c, d, e, f, g, h, T1, T2;
            let W = new Array(16);
            a = ihash[0];
            b = ihash[1];
            c = ihash[2];
            d = ihash[3];
            e = ihash[4];
            f = ihash[5];
            g = ihash[6];
            h = ihash[7];
            for (let i = 0; i < 16; i++) W[i] = buffer[(i << 2) + 3] | (buffer[(i << 2) + 2] << 8) | (buffer[(i << 2) + 1] << 16) | (buffer[i << 2] << 24);
            for (let j = 0; j < 64; j++) {
                T1 = h + sha256_Sigma1(e) + choice(e, f, g) + K256[j];
                if (j < 16) T1 += W[j];
                else T1 += sha256_expand(W, j);
                T2 = sha256_Sigma0(a) + majority(a, b, c);
                h = g;
                g = f;
                f = e;
                e = safe_add(d, T1);
                d = c;
                c = b;
                b = a;
                a = safe_add(T1, T2);
            }
            ihash[0] += a;
            ihash[1] += b;
            ihash[2] += c;
            ihash[3] += d;
            ihash[4] += e;
            ihash[5] += f;
            ihash[6] += g;
            ihash[7] += h;
        }

        let sha256_update = (data, inputLen) => {
            let i, index, curpos = 0;
            index = (count[0] >> 3) & 0x3f;
            let remainder = inputLen & 0x3f;
            if ((count[0] += inputLen << 3) < inputLen << 3) count[1]++;
            count[1] += inputLen >> 29;
            if (typeof data == "string") {
                for (i = 0; i + 63 < inputLen; i += 64) {
                    for (let j = index; j < 64; j++) buffer[j] = data.charCodeAt(curpos++);
                    sha256_transform();
                    index = 0;
                }
                for (let j = 0; j < remainder; j++) buffer[j] = data.charCodeAt(curpos++);
            }
            else {
                for (i = 0; i + 63 < inputLen; i += 64) {
                    for (let j = index; j < 64; j++) buffer[j] = data[curpos++];
                    sha256_transform();
                    index = 0;
                }
                for (let j = 0; j < remainder; j++) buffer[j] = data[curpos++];
            }

        }

        let sha256_final = () => {
            let index = (count[0] >> 3) & 0x3f;
            buffer[index++] = 0x80;
            if (index <= 56) { for (let i = index; i < 56; i++) buffer[i] = 0 } else {
                for (let i = index; i < 64; i++) buffer[i] = 0;
                sha256_transform();
                for (let i = 0; i < 56; i++) buffer[i] = 0;
            }
            buffer[56] = (count[1] >>> 24) & 0xff;
            buffer[57] = (count[1] >>> 16) & 0xff;
            buffer[58] = (count[1] >>> 8) & 0xff;
            buffer[59] = count[1] & 0xff;
            buffer[60] = (count[0] >>> 24) & 0xff;
            buffer[61] = (count[0] >>> 16) & 0xff;
            buffer[62] = (count[0] >>> 8) & 0xff;
            buffer[63] = count[0] & 0xff;
            sha256_transform();
        }

        let sha256_encode_bytes = () => {
            let j = 0;
            let output = new Array(32);
            for (let i = 0; i < 8; i++) {
                output[j++] = (ihash[i] >>> 24) & 0xff;
                output[j++] = (ihash[i] >>> 16) & 0xff;
                output[j++] = (ihash[i] >>> 8) & 0xff;
                output[j++] = ihash[i] & 0xff;
            }
            return output;
        }

        let sha256_encode_hex = () => { let output = new String(); for (let i = 0; i < 8; i++) { for (let j = 28; j >= 0; j -= 4) output += sha256_hex_digits.charAt((ihash[i] >>> j) & 0x0f) } return output; }

        let hash = (data, bytes, gen) => {
            sha256_init();
            sha256_update(data, data.length);
            sha256_final();
            if (bytes) {
                return sha256_encode_bytes();
            } else {
                return sha256_encode_hex();
            }
        };
        getHash = hash;

    })();

    userName = getHash(userName);
    password = getHash(password);

    // get a unix epoch timestamp is millisceonds
    // 获取一个unix时间戳，毫秒
    let t = new Date().getTime().toString();

    // use user name(SHA256) + password(SHA256) + timestamp(string format)
    // to generate a hash using SHA256
    // 使用 用户名(SHA256) + 密码(SHA256) + 时间戳(字符串格式)
    // 使用SHA256生成一个数字摘要
    let hash = getHash(userName + password + t);

    let host = `http://${domain}:${port}`;

    let header = host + "/exec_provider?";

    // confirm is optional
    // confirm是可选的
    // the number of confirm could be anything(temporary, it may be changed as other purpose)
    // confirm的数字随便写(暂时，它可能会改成其他用途)
    let queryString = `${header}tid=${targetID}&cpid=${cpid}&t=` + t + "&hash=" + hash + "&confirm=100";

    // use GET method to execute provider
    // 使用GET方法执行provider
    require("http").get(queryString, function (res) {
        let data = "";
        res.on("data", function (d) {
            data += d.toString();
        });
        res.on("close", function () {
            // server will return a random id(SHA256 hex string)
            // that indecated your arguments is right
            // this id may changed for other purpose
            // 服务器会返回一个随机的id，SHA256 16进制字符串
            // 这表示你的请求参数正确
            // 这个id可能会用作其他用途
            console.log(data);
        });
    });
})();