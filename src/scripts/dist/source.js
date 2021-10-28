/******/ (function() { // webpackBootstrap
/******/ 	"use strict";
/******/ 	// The require scope
/******/ 	var __webpack_require__ = {};
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/define property getters */
/******/ 	!function() {
/******/ 		// define getter functions for harmony exports
/******/ 		__webpack_require__.d = function(exports, definition) {
/******/ 			for(var key in definition) {
/******/ 				if(__webpack_require__.o(definition, key) && !__webpack_require__.o(exports, key)) {
/******/ 					Object.defineProperty(exports, key, { enumerable: true, get: definition[key] });
/******/ 				}
/******/ 			}
/******/ 		};
/******/ 	}();
/******/ 	
/******/ 	/* webpack/runtime/hasOwnProperty shorthand */
/******/ 	!function() {
/******/ 		__webpack_require__.o = function(obj, prop) { return Object.prototype.hasOwnProperty.call(obj, prop); }
/******/ 	}();
/******/ 	
/******/ 	/* webpack/runtime/make namespace object */
/******/ 	!function() {
/******/ 		// define __esModule on exports
/******/ 		__webpack_require__.r = function(exports) {
/******/ 			if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 				Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 			}
/******/ 			Object.defineProperty(exports, '__esModule', { value: true });
/******/ 		};
/******/ 	}();
/******/ 	
/************************************************************************/
var __webpack_exports__ = {};
// ESM COMPAT FLAG
__webpack_require__.r(__webpack_exports__);

// EXPORTS
__webpack_require__.d(__webpack_exports__, {
  "info": function() { return /* binding */ info; },
  "nextQuery": function() { return /* binding */ nextQuery; },
  "processResponse": function() { return /* binding */ processResponse; },
  "query": function() { return /* binding */ query; }
});

;// CONCATENATED MODULE: ./protocols/lib/data-reader.ts
var getUint64BigInt = function (dataview, byteOffset, littleEndian) {
    var left = dataview.getUint32(byteOffset, littleEndian);
    var right = dataview.getUint32(byteOffset + 4, littleEndian);
    return littleEndian
        ? left + Math.pow(2, 32) * right
        : Math.pow(2, 32) * left + right;
};
var DataReader = /** @class */ (function () {
    function DataReader(_buf) {
        this._buf = _buf;
        this.pos = 0;
        this._view = new DataView(_buf.buffer);
    }
    DataReader.prototype.skip = function (length) {
        this.pos += length;
        return this;
    };
    DataReader.prototype.u8 = function () {
        var ret = this._view.getUint8(this.pos);
        this.pos += 1;
        return ret;
    };
    DataReader.prototype.u16le = function () {
        var ret = this._view.getUint16(this.pos, true);
        this.pos += 2;
        return ret;
    };
    DataReader.prototype.u32le = function () {
        var ret = this._view.getUint32(this.pos, true);
        this.pos += 4;
        return ret;
    };
    DataReader.prototype.u64le = function () {
        var ret = getUint64BigInt(this._view, this.pos, true);
        this.pos += 8;
        return ret;
    };
    DataReader.prototype.i8 = function () {
        var ret = this._view.getInt8(this.pos);
        this.pos += 1;
        return ret;
    };
    DataReader.prototype.i16le = function () {
        var ret = this._view.getInt16(this.pos, true);
        this.pos += 2;
        return ret;
    };
    DataReader.prototype.i32le = function () {
        var ret = this._view.getInt32(this.pos, true);
        this.pos += 4;
        return ret;
    };
    DataReader.prototype.f32le = function () {
        var ret = this._view.getFloat32(this.pos, true);
        this.pos += 4;
        return ret;
    };
    DataReader.prototype.f64le = function () {
        var ret = this._view.getFloat64(this.pos, true);
        this.pos += 8;
        return ret;
    };
    DataReader.prototype.zstring = function () {
        var idx = Array.prototype.indexOf.call(this._buf, 0, this.pos);
        if (idx == -1) {
            return this.lstring(this._buf.length - this.pos);
        }
        return this.lstring(idx - this.pos + 1);
    };
    DataReader.prototype.lstring = function (length) {
        var decoder = new TextDecoder('utf8');
        var offset = this._view.byteOffset + this.pos;
        var ret = decoder.decode(new DataView(this._view.buffer, offset, length));
        this.pos += length;
        return ret;
    };
    DataReader.prototype.data = function () {
        var ret = this._buf.slice(this.pos);
        this.pos += ret.byteLength;
        return ret;
    };
    return DataReader;
}());


;// CONCATENATED MODULE: ./protocols/lib/data-writer.ts
var DataWriter = /** @class */ (function () {
    function DataWriter() {
        this.pos = 0;
        this._buf = new Uint8Array(256);
        this._view = new DataView(this._buf.buffer);
    }
    Object.defineProperty(DataWriter.prototype, "buf", {
        get: function () {
            return this._buf.subarray(0, this.pos);
        },
        enumerable: false,
        configurable: true
    });
    DataWriter.prototype.u8 = function (val) {
        this._view.setUint8(this.pos, val);
        this.pos += 1;
        return this;
    };
    DataWriter.prototype.u16le = function (val) {
        this._view.setUint16(this.pos, val, true);
        this.pos += 2;
        return this;
    };
    DataWriter.prototype.u32le = function (val) {
        this._view.setUint32(this.pos, val, true);
        this.pos += 4;
        return this;
    };
    DataWriter.prototype.u64le = function (val) {
        this._view.setBigUint64(this.pos, val, true);
        this.pos += 8;
        return this;
    };
    DataWriter.prototype.i8 = function (val) {
        this._view.setInt8(this.pos, val);
        this.pos += 1;
        return this;
    };
    DataWriter.prototype.i16le = function (val) {
        this._view.setInt16(this.pos, val, true);
        this.pos += 2;
        return this;
    };
    DataWriter.prototype.i32le = function (val) {
        this._view.setInt32(this.pos, val, true);
        this.pos += 4;
        return this;
    };
    DataWriter.prototype.i64le = function (val) {
        this._view.setBigInt64(this.pos, val, true);
        this.pos += 8;
        return this;
    };
    DataWriter.prototype.f32le = function (val) {
        this._view.setFloat32(this.pos, val, true);
        this.pos += 4;
        return this;
    };
    DataWriter.prototype.f64le = function (val) {
        this._view.setFloat64(this.pos, val, true);
        this.pos += 8;
        return this;
    };
    DataWriter.prototype.zstring = function (val) {
        this.lstring(val, val.length);
        this.u8(0);
        return this;
    };
    DataWriter.prototype.lstring = function (val, length) {
        var encoder = new TextEncoder();
        var buf = encoder.encode(val);
        this.data(buf.subarray(0, length));
        return this;
    };
    DataWriter.prototype.data = function (data) {
        this._buf.set(data, this.pos);
        this.pos += data.length;
        return this;
    };
    return DataWriter;
}());


;// CONCATENATED MODULE: ./protocols/source.ts


var info = {
    name: 'Source Engine',
    version: '1.0',
    transport: 'udp',
};
var requests = {};
var gotChallenge = -1;
var gotServerInfo = null;
var gotPlayerList = null;
var games = {
    0: function (inf) {
        if (inf.dir === 'cstrike') {
            return {
                gameId: 'cs',
                gameName: 'Counter-Strike',
            };
        }
        else {
            return {};
        }
    },
    10: function (inf) {
        if (inf.dir === 'cstrike') {
            return {
                gameId: 'cs',
                gameName: 'Counter-Strike',
            };
        }
        else {
            return {
                gameId: 'hl',
                gameName: 'Half-Life',
            };
        }
    },
    20: {
        gameId: 'tf',
        gameName: 'Team Fortress',
    },
    30: {
        gameId: 'dod',
        gameName: 'Day of Defeat',
    },
    80: {
        gameId: 'czero',
        gameName: 'Counter-Strike: Condition Zero',
    },
    240: {
        gameId: 'css',
        gameName: 'Counter-Strike: Source',
    },
    300: {
        gameId: 'dods',
        gameName: 'Day of Defeat: Source',
    },
    320: {
        gameId: 'hl2dm',
        gameName: 'Half-Life 2: Deathmatch',
    },
    440: {
        gameId: 'tf2',
        gameName: 'Team Fortress 2',
    },
    500: function (inf) { return ({
        gameId: 'l4d',
        gameName: 'Left 4 Dead',
        gameMode: inf.desc.substr(6),
    }); },
    550: function (inf) {
        var modes = {
            coop: 'Co-op',
            realism: 'Realism',
            survival: 'Survival',
            versus: 'Versus',
            scavenge: 'Scavenge',
        };
        return {
            gameId: 'l4d2',
            gameName: 'Left 4 Dead ',
            gameMode: modes[inf.keywords || 'coop'],
        };
    },
    630: {
        gameId: 'as',
        gameName: 'Alien Swarm',
    },
    730: {
        gameId: 'csgo',
        gameName: 'Counter-Strike: Global Offensive',
    },
    17500: {
        gameId: 'zp',
        gameName: 'Zombie Panic!',
    },
    17520: {
        gameId: 'syn',
        gameName: 'Synergy',
    },
    17700: {
        gameId: 'ins',
        gameName: 'Insurgency',
    },
    17710: {
        gameId: 'nd',
        gameName: 'Nuclear Dawn',
    },
    41070: {
        gameId: 'ss3',
        gameName: 'Serious Sam 3',
    },
};
var sendPacket = function (type, payload) {
    var w = new DataWriter();
    w.i32le(-1);
    w.lstring(type, 1);
    if (payload) {
        w.data(payload);
    }
    gsw.send(w.buf);
};
var sendServerInfoPacket = function (challenge) {
    var w = new DataWriter();
    w.zstring('Source Engine Query');
    if (challenge) {
        w.i32le(challenge);
    }
    sendPacket('T', w.buf);
};
var sendPlayerListPacket = function (challenge) {
    var w = new DataWriter();
    if (challenge) {
        w.i32le(challenge);
    }
    sendPacket('U', w.buf);
};
var nextQuery = function () {
    if (!gotServerInfo) {
        sendServerInfoPacket(gotChallenge);
        return;
    }
    if (!gotPlayerList) {
        sendPlayerListPacket(gotChallenge);
    }
};
var query = function () {
    gotChallenge = -1;
    gotServerInfo = null;
    gotPlayerList = null;
    nextQuery();
};
var normalizeServerInfo = function (inf) {
    var ret = {
        gameId: 'unknown',
        gameName: 'Unknown Game',
        name: inf.name,
        map: inf.map,
        numPlayers: inf.numPlayers,
        maxPlayers: inf.maxPlayers,
        version: inf.version,
        private: inf.private,
        secure: inf.secure,
    };
    var getMoreInfo = games[inf.appid || 0] || games[0];
    if (getMoreInfo) {
        var moreInfo = typeof getMoreInfo === 'function'
            ? getMoreInfo(inf)
            : getMoreInfo;
        if (moreInfo) {
            Object.assign(ret, moreInfo);
        }
    }
    return ret;
};
var readServerInfoGold = function (r) {
    var ret = {
        gameId: 'unknown',
        gameName: 'Unknown Game',
        address: r.zstring(),
        name: r.zstring(),
        map: r.zstring(),
        dir: r.zstring(),
        desc: r.zstring(),
        numPlayers: r.u8(),
        maxPlayers: r.u8(),
        numBots: 0,
        protocolVersion: r.u8(),
        serverType: r.lstring(1),
        osType: r.lstring(1),
        private: Boolean(r.u8()),
    };
    var mod = r.u8();
    if (ret.mod) {
        ret.mod = {
            link: r.zstring(),
            download: r.zstring(),
            null: r.u8(),
            version: r.i32le(),
            size: r.i32le(),
        };
    }
    ret.secure = Boolean(r.u8());
    ret.numBots = r.u8();
    return ret;
};
var readServerInfo = function (r) {
    var inf = {
        gameId: 'unknown',
        gameName: 'Unknown Game',
        protocolVersion: r.u8(),
        name: r.zstring(),
        map: r.zstring(),
        dir: r.zstring(),
        desc: r.zstring(),
        appid: r.u16le(),
        numPlayers: r.u8(),
        maxPlayers: r.u8(),
        numBots: r.u8(),
        serverType: r.lstring(1),
        osType: r.lstring(1),
        private: Boolean(r.u8()),
        secure: Boolean(r.u8()),
    };
    if (inf.appid === 2400) { // The Ship
        inf.theShip = {
            mode: r.u8(),
            witnesses: r.u8(),
            duration: r.u8(),
        };
    }
    inf.version = r.zstring();
    var edf = r.u8();
    if (edf & 0x80) {
        inf.serverPort = r.u16le();
    }
    if (edf & 0x10) {
        gsw.print('warn: need to read \'inf.steamid\'');
        // inf.steamid = r.u64le()
        r.skip(8);
    }
    if (edf & 0x40) {
        inf.sourcetvPort = r.u16le();
        inf.sourcetvName = r.zstring();
    }
    if (edf & 0x20) {
        inf.keywords = r.zstring();
    }
    if (edf & 0x01) {
        gsw.print('warn: need to read \'inf.gameId\'');
        // inf.gameId = r.i64le()
        r.skip(8);
    }
    return inf;
};
var readPlayerList = function (r, inf) {
    var players = [];
    var count = r.u8();
    for (var i = 0; i < count; i++) {
        var player = {
            index: r.u8(),
            name: r.zstring(),
            score: r.u32le(),
            duration: r.f32le(),
        };
        if (inf && inf.appid == 2400) { // The Ship
            player.deaths = r.u32le();
            player.money = r.u32le();
            players.push(player);
        }
    }
    return players;
};
var readChallenge = function (r) {
    return r.i32le();
};
var readPayload = function (r) {
    var type = r.lstring(1);
    switch (type) {
        case 'I':
            gotServerInfo = readServerInfo(r);
            gotChallenge = -1;
            gsw.sinfo(normalizeServerInfo(gotServerInfo));
            nextQuery();
            break;
        case 'm':
            gotServerInfo = readServerInfoGold(r);
            gotChallenge = -1;
            gsw.sinfo(normalizeServerInfo(gotServerInfo));
            nextQuery();
            break;
        case 'D':
            gotPlayerList = readPlayerList(r, gotServerInfo);
            gotChallenge = -1;
            gsw.plist(gotPlayerList);
            nextQuery();
            break;
        case 'A':
            gotChallenge = readChallenge(r);
            nextQuery();
    }
};
var readHeaderGold = function (r) {
    var reqid = r.u32le();
    var number = r.u8();
    var total = number & 0x0f;
    number >>= 4;
    if (!reqid) {
        throw new Error('Incorrect reqid');
    }
    if (!total) {
        throw new Error('Incorrect total');
    }
    if (number >= total) {
        throw new Error('Incorrect number');
    }
    var payload = r.data();
    return { reqid: reqid, total: total, number: number, payload: payload };
};
var readHeader = function (r) {
    var reqid = r.u32le();
    var total = r.u8();
    var number = r.u8();
    var size = r.u16le();
    var compressed = Boolean(reqid >> 31);
    var dataSize, crc32;
    if (!reqid) {
        throw new Error('Incorrect reqid');
    }
    if (!total) {
        throw new Error('Incorrect total');
    }
    if (number >= total) {
        throw new Error('Incorrect number');
    }
    if (number === 0 && compressed) {
        dataSize = r.u32le();
        crc32 = r.u32le();
    }
    var payload = r.data();
    return { reqid: reqid, total: total, number: number, compressed: compressed, dataSize: dataSize, crc32: crc32, payload: payload };
};
var tryReadHeader = function (r) {
    var pos = r.pos;
    var pak = readHeaderGold(r);
    if (!pak) {
        r.pos = pos;
        pak = readHeader(r);
    }
    return pak;
};
var storePacket = function (pak) {
    if (!requests[pak.reqid]) {
        requests[pak.reqid] = {
            packets: [],
            firstPacketTime: Date.now(),
            total: pak.total,
        };
    }
    var req = requests[pak.reqid];
    if (req.total !== pak.total) {
        throw new Error('Incorrect packet');
    }
    req.packets[pak.number] = pak.payload;
    return req;
};
var gotAllPackets = function (req) {
    return req.packets.every(function (pak) { return pak; });
};
var concatBuffers = function (bufs) {
    var size = bufs.reduce(function (acc, buf) { return acc + buf.byteLength; }, 0);
    var result = new Uint8Array(size);
    var offset = 0;
    bufs.forEach(function (buf) {
        result.set(buf, offset);
        offset += buf.byteLength;
    });
    return result;
};
var processResponse = function (data) {
    var r = new DataReader(data);
    var format = r.i32le();
    if (format === -1) {
        readPayload(r);
    }
    else if (format == -2) {
        var pak = tryReadHeader(r);
        if (!pak) {
            throw new Error('Could not read packet header response');
        }
        if (pak.compressed) {
            throw new Error('Compressed split packets are not supported');
        }
        var req = storePacket(pak);
        if (gotAllPackets(req)) {
            var data_1 = concatBuffers(req.packets);
            var r_1 = new DataReader(data_1);
            readPayload(r_1);
            delete requests[pak.reqid];
        }
    }
};

globalThis.module = __webpack_exports__;
/******/ })()
;