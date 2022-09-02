module.exports = function (blacklist, visitors, getType, print, fnProvider) {
    let url = require("url");
    let http = require("http");
    let fs = require("fs");
    let zlib = require("zlib");
    return http.createServer(function (req, res) {

        //parse url
        //解析url
        let urlObj = url.parse(req.url, true);
        let pathName = urlObj.pathname;

        //default page
        //默认页面
        if (pathName == "/") {
            pathName = "index.html";
        } else {
            pathName = pathName.substring(1);
        }

        //this part is same as iot.js
        //这部分和iot.js相同
        let ip = /\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}/.exec(req.socket.remoteAddress);

        if (!ip) {
            res.destroy();
            print("a hacker detected 0");
            return;
        }

        ip = ip.toString();

        if (getType(ip) != "string") {
            res.destroy();
            print("a hacker detected 1");
            return;
        }

        if (!ip.length) {
            res.destroy();
            print("a hacker detected 2");
            return;
        }

        let hacker = blacklist.find(e => { return e == ip; });
        if (hacker) {
            res.destroy();
            print("a hacker detected 3");
            return;
        }

        let visitor = visitors.find(e => { return e.ip == ip; });
        if (visitor) {
            if (visitor.times++ > 100) {
                res.destroy();
                blacklist.push(ip);
                print("a hacker detected");
                return;
            }
        } else {
            visitors.push({
                ip: ip,
                times: 0
            });
        }


        //这个是以前用的现在已经不用了，暂时先不删
        if (/showtime/.test(pathName)) {
            res.writeHeader(200);
            let t = new Date();
            let hour = t.getHours().toString();
            hour = hour.length == 1 ? "0" + hour : hour;
            let min = t.getMinutes().toString();
            min = min.length == 1 ? "0" + min : min;
            res.end(hour + min);
            return;
        }

        //not allow request js, json and node_modules
        //不允许请求 js, json 和 node.js 的组件目录
        if (/\.js/.test(pathName)) {
            res.writeHead(404);
            res.end();
            return;
        }
        if (/\.json/.test(pathName)) {
            res.writeHead(404);
            res.end();
            return;
        }
        if (/node_modules/.test(pathName)) {
            res.writeHead(404);
            res.end();
            return;
        }

        //when use http get method to execute provider
        //当使用http get方法请求执行provider时
        if (/exec_provider/.test(pathName)) {
            let fromID = fnProvider(urlObj.query);

            //if function return ""
            //means wait for response
            //如果函数返回空字符串
            //表示需要等待返回值
            if (fromID.length)
                res.end(fromID);
            return;
        }

        let stat = fs.statSync(pathName, {
            throwIfNoEntry: false
        });
        if (!stat) {
            res.writeHead(404);
            res.end();
            return;
        }
        let tFromLocal = new Date(stat.mtime);

        let sendFile = e => {
            res.setHeader("Last-Modified", tFromLocal.toGMTString());
            res.writeHead(200, { 'Content-Encoding': 'gzip' });
            res.end(zlib.gzipSync(fs.readFileSync(pathName)));
        };


        if (req.headers["if-modified-since"]) {

            let tFromRemote = new Date(req.headers["if-modified-since"]);
            if (tFromRemote) {
                tFromRemote = tFromRemote.getTime() + 1000;
            }

            if (tFromRemote < tFromLocal.getTime()) {
                sendFile();
            } else {
                res.writeHeader(304);
                res.write("Not Modified");
                res.end();
            }

        } else {
            sendFile();
        }

    });
};