(function(){function e(e){for(let t=0;t<e.byteLength;++t)e.arr.push(0);let t=new Uint8Array(e.arr),n=new DataView(t.buffer);switch(e.type){case"8":n.setUint8(t.byteLength-e.byteLength,e.number);break;case"16":n.setUint16(t.byteLength-e.byteLength,e.number,!0);break;case"32":n.setUint32(t.byteLength-e.byteLength,e.number,!0);break;case"64":n.setBigUint64(t.byteLength-e.byteLength,BigInt(e.number),!0);break;case"f":n.setFloat32(t.byteLength-e.byteLength,e.number,!0);break;case"d":n.setFloat64(t.byteLength-e.byteLength,e.number,!0)}return e.arr=Array.from(t),e.arr}function t(t){if(!t)return;if(!t.length)return;let n=Object.prototype.toString,o=[];for(let L=0;L<t.length;L++)if(typeName=n.call(t[L]).toLowerCase().split(" ")[1],0<=typeName.indexOf("number")){let n=t[L],u=n.toString();if(parseInt(u)==parseFloat(u))n>=0?n<256?(o.push(r),o=e({arr:o,byteLength:1,number:n,type:"8"})):n>255&&n<65536?(o.push(a),o=e({arr:o,byteLength:2,number:n,type:"16"})):n>65535&&n<4294967296?(o.push(g),o=e({arr:o,byteLength:4,number:n,type:"32"})):(o.push(h),o=e({arr:o,byteLength:8,number:n,type:"64"})):n>=-128?(o.push(b),o=e({arr:o,byteLength:1,number:n,type:"8"})):n<-128&&n>=-32768?(o.push(f),o=e({arr:o,byteLength:2,number:n,type:"16"})):n<-32768&&n>=-2147483648?(o.push(i),o=e({arr:o,byteLength:4,number:n,type:"32"})):(o.push(y),o=e({arr:o,byteLength:8,number:n,type:"64"}));else{let t=n.toString().length-1;t>5?(o.push(l),o=e({arr:o,byteLength:8,number:n,type:"d"})):(o.push(p),o=e({arr:o,byteLength:4,number:n,type:"f"}))}}else if(0<=typeName.indexOf("bigint"))o.push(h),o=e({arr:o,byteLength:8,number:t[L],type:"64"});else if(0<=typeName.indexOf("uint8array")){o.push(u),o=e({arr:o,byteLength:4,number:t[L].length,type:"32"});for(let e of t[L])o.push(e)}else if(0<=typeName.indexOf("string")){o.push(s);let n=(new TextEncoder).encode(t[L]);o=e({arr:o,byteLength:4,number:n.length+1,type:"32"});for(let e of n)o.push(e);o.push(0)}return new Uint8Array(o).buffer}function n(e,t,n,o,L){if(!e)return;if(!e.byteLength)return;let c,m=[],d=e=>n?{}:[];try{c=new DataView(e)}catch(e){return d()}let U=0;for(;U<e.byteLength;){let t=c.getUint8(U);if(m.length&&o)break;switch(t){case r:case b:if(++U,e.byteLength-U<1)return d();if(L?m.push({data:t==r?c.getUint8(U):c.getInt8(U),offset:U,length:1}):m.push(t==r?c.getUint8(U):c.getInt8(U)),++U,o)break;break;case a:case f:if(++U,e.byteLength-U<2)return d();L?m.push({data:t==a?c.getUint16(U,!0):c.getInt16(U,!0),offset:U,length:2}):m.push(t==a?c.getUint16(U,!0):c.getInt16(U,!0)),U+=2;break;case g:case i:if(++U,e.byteLength-U<4)return d();L?m.push({data:t==g?c.getUint32(U,!0):c.getInt32(U,!0),offset:U,length:4}):m.push(t==g?c.getUint32(U,!0):c.getInt32(U,!0)),U+=4;break;case h:case y:if(++U,e.byteLength-U<8)return d();L?m.push({data:t==h?c.getBigUint64(U,!0):c.getBigInt64(U,!0),offset:U,length:8}):m.push(t==h?c.getBigUint64(U,!0):c.getBigInt64(U,!0)),U+=8;break;case p:case l:if(++U,e.byteLength-U<(t==p?4:8))return d();L?m.push({data:m.push(t==p?c.getFloat32(U,!0):c.getFloat64(U,!0)),offset:U,length:t==p?4:8}):m.push(t==p?c.getFloat32(U,!0):c.getFloat64(U,!0)),U+=t==p?4:8;break;case u:if(++U,e.byteLength-U<4)return d();let n=c.getUint32(U,!0);if(U+=4,e.byteLength-U<n)return d();if(L&&n+U>e.byteLength){m.push({offset:U,length:n});break}let w=new Uint8Array(n);for(let e=0;e<n;e++)w[e]=c.getUint8(U++);L?m.push({data:w,offset:U-n-1,length:n}):m.push(w);break;case s:if(++U,e.byteLength-U<4)return d();let k=c.getUint32(U,!0)-1;if(U+=4,e.byteLength-U<k)return d();let B=new Uint8Array(k);for(let e=0;e<k;e++)B[e]=c.getUint8(U++);B=(new TextDecoder).decode(B),++U,L?m.push({data:B,offset:U-k-1,length:k}):m.push(B)}}if(!n)return m;if(!n.length)return m;let w={};for(let e=0;e<n.length;e++)e>m.length-1?w[n[e]]=null:w[n[e]]=m[e];return w}const r=128,a=129,g=130,h=131,u=133,s=134,b=136,f=137,i=144,y=145,p=146,l=147;"object"==typeof window?(window.createArrayBuffer=t,window.decodeArrayBuffer=n):module.exports={createArrayBuffer:t,decodeArrayBuffer:n}})();