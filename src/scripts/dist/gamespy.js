/******/ (function() { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ 213:
/*!********************************!*\
  !*** ./lib/polyfill/buffer.ts ***!
  \********************************/
/***/ (function() {


Buffer.alloc = function alloc(size, fill, encoding) {
    var buf = new Buffer(size);
    if (fill != null) {
        buf.fill(fill, 0, buf.length, encoding);
    }
    return buf;
};
// @ts-expect-error
Buffer.from = function from(str, encoding) {
    return new Buffer(str, encoding);
};


/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
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
// This entry need to be wrapped in an IIFE because it need to be isolated against other modules in the chunk.
!function() {
/*!******************************************!*\
  !*** ./protocols/gamespy.ts + 7 modules ***!
  \******************************************/
// ESM COMPAT FLAG
__webpack_require__.r(__webpack_exports__);

// EXPORTS
__webpack_require__.d(__webpack_exports__, {
  "info": function() { return /* binding */ gamespy_info; },
  "processResponse": function() { return /* binding */ processResponse; },
  "query": function() { return /* binding */ query; }
});

;// CONCATENATED MODULE: ./node_modules/tslib/tslib.es6.js
/*! *****************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */
/* global Reflect, Promise */
var extendStatics = function (d, b) {
    extendStatics = Object.setPrototypeOf ||
        ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
        function (d, b) { for (var p in b)
            if (Object.prototype.hasOwnProperty.call(b, p))
                d[p] = b[p]; };
    return extendStatics(d, b);
};
function __extends(d, b) {
    if (typeof b !== "function" && b !== null)
        throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
    extendStatics(d, b);
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
}
var __assign = function () {
    __assign = Object.assign || function __assign(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s)
                if (Object.prototype.hasOwnProperty.call(s, p))
                    t[p] = s[p];
        }
        return t;
    };
    return __assign.apply(this, arguments);
};
function __rest(s, e) {
    var t = {};
    for (var p in s)
        if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
            t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
        for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
            if (e.indexOf(p[i]) < 0 && Object.prototype.propertyIsEnumerable.call(s, p[i]))
                t[p[i]] = s[p[i]];
        }
    return t;
}
function __decorate(decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function")
        r = Reflect.decorate(decorators, target, key, desc);
    else
        for (var i = decorators.length - 1; i >= 0; i--)
            if (d = decorators[i])
                r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
}
function __param(paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); };
}
function __metadata(metadataKey, metadataValue) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function")
        return Reflect.metadata(metadataKey, metadataValue);
}
function __awaiter(thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try {
            step(generator.next(value));
        }
        catch (e) {
            reject(e);
        } }
        function rejected(value) { try {
            step(generator["throw"](value));
        }
        catch (e) {
            reject(e);
        } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
}
function __generator(thisArg, body) {
    var _ = { label: 0, sent: function () { if (t[0] & 1)
            throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function () { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f)
            throw new TypeError("Generator is already executing.");
        while (_)
            try {
                if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done)
                    return t;
                if (y = 0, t)
                    op = [op[0] & 2, t.value];
                switch (op[0]) {
                    case 0:
                    case 1:
                        t = op;
                        break;
                    case 4:
                        _.label++;
                        return { value: op[1], done: false };
                    case 5:
                        _.label++;
                        y = op[1];
                        op = [0];
                        continue;
                    case 7:
                        op = _.ops.pop();
                        _.trys.pop();
                        continue;
                    default:
                        if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) {
                            _ = 0;
                            continue;
                        }
                        if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) {
                            _.label = op[1];
                            break;
                        }
                        if (op[0] === 6 && _.label < t[1]) {
                            _.label = t[1];
                            t = op;
                            break;
                        }
                        if (t && _.label < t[2]) {
                            _.label = t[2];
                            _.ops.push(op);
                            break;
                        }
                        if (t[2])
                            _.ops.pop();
                        _.trys.pop();
                        continue;
                }
                op = body.call(thisArg, _);
            }
            catch (e) {
                op = [6, e];
                y = 0;
            }
            finally {
                f = t = 0;
            }
        if (op[0] & 5)
            throw op[1];
        return { value: op[0] ? op[1] : void 0, done: true };
    }
}
var __createBinding = Object.create ? (function (o, m, k, k2) {
    if (k2 === undefined)
        k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function () { return m[k]; } });
}) : (function (o, m, k, k2) {
    if (k2 === undefined)
        k2 = k;
    o[k2] = m[k];
});
function __exportStar(m, o) {
    for (var p in m)
        if (p !== "default" && !Object.prototype.hasOwnProperty.call(o, p))
            __createBinding(o, m, p);
}
function __values(o) {
    var s = typeof Symbol === "function" && Symbol.iterator, m = s && o[s], i = 0;
    if (m)
        return m.call(o);
    if (o && typeof o.length === "number")
        return {
            next: function () {
                if (o && i >= o.length)
                    o = void 0;
                return { value: o && o[i++], done: !o };
            }
        };
    throw new TypeError(s ? "Object is not iterable." : "Symbol.iterator is not defined.");
}
function __read(o, n) {
    var m = typeof Symbol === "function" && o[Symbol.iterator];
    if (!m)
        return o;
    var i = m.call(o), r, ar = [], e;
    try {
        while ((n === void 0 || n-- > 0) && !(r = i.next()).done)
            ar.push(r.value);
    }
    catch (error) {
        e = { error: error };
    }
    finally {
        try {
            if (r && !r.done && (m = i["return"]))
                m.call(i);
        }
        finally {
            if (e)
                throw e.error;
        }
    }
    return ar;
}
/** @deprecated */
function __spread() {
    for (var ar = [], i = 0; i < arguments.length; i++)
        ar = ar.concat(__read(arguments[i]));
    return ar;
}
/** @deprecated */
function __spreadArrays() {
    for (var s = 0, i = 0, il = arguments.length; i < il; i++)
        s += arguments[i].length;
    for (var r = Array(s), k = 0, i = 0; i < il; i++)
        for (var a = arguments[i], j = 0, jl = a.length; j < jl; j++, k++)
            r[k] = a[j];
    return r;
}
function __spreadArray(to, from, pack) {
    if (pack || arguments.length === 2)
        for (var i = 0, l = from.length, ar; i < l; i++) {
            if (ar || !(i in from)) {
                if (!ar)
                    ar = Array.prototype.slice.call(from, 0, i);
                ar[i] = from[i];
            }
        }
    return to.concat(ar || Array.prototype.slice.call(from));
}
function __await(v) {
    return this instanceof __await ? (this.v = v, this) : new __await(v);
}
function __asyncGenerator(thisArg, _arguments, generator) {
    if (!Symbol.asyncIterator)
        throw new TypeError("Symbol.asyncIterator is not defined.");
    var g = generator.apply(thisArg, _arguments || []), i, q = [];
    return i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i;
    function verb(n) { if (g[n])
        i[n] = function (v) { return new Promise(function (a, b) { q.push([n, v, a, b]) > 1 || resume(n, v); }); }; }
    function resume(n, v) { try {
        step(g[n](v));
    }
    catch (e) {
        settle(q[0][3], e);
    } }
    function step(r) { r.value instanceof __await ? Promise.resolve(r.value.v).then(fulfill, reject) : settle(q[0][2], r); }
    function fulfill(value) { resume("next", value); }
    function reject(value) { resume("throw", value); }
    function settle(f, v) { if (f(v), q.shift(), q.length)
        resume(q[0][0], q[0][1]); }
}
function __asyncDelegator(o) {
    var i, p;
    return i = {}, verb("next"), verb("throw", function (e) { throw e; }), verb("return"), i[Symbol.iterator] = function () { return this; }, i;
    function verb(n, f) { i[n] = o[n] ? function (v) { return (p = !p) ? { value: __await(o[n](v)), done: n === "return" } : f ? f(v) : v; } : f; }
}
function __asyncValues(o) {
    if (!Symbol.asyncIterator)
        throw new TypeError("Symbol.asyncIterator is not defined.");
    var m = o[Symbol.asyncIterator], i;
    return m ? m.call(o) : (o = typeof __values === "function" ? __values(o) : o[Symbol.iterator](), i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i);
    function verb(n) { i[n] = o[n] && function (v) { return new Promise(function (resolve, reject) { v = o[n](v), settle(resolve, reject, v.done, v.value); }); }; }
    function settle(resolve, reject, d, v) { Promise.resolve(v).then(function (v) { resolve({ value: v, done: d }); }, reject); }
}
function __makeTemplateObject(cooked, raw) {
    if (Object.defineProperty) {
        Object.defineProperty(cooked, "raw", { value: raw });
    }
    else {
        cooked.raw = raw;
    }
    return cooked;
}
;
var __setModuleDefault = Object.create ? (function (o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function (o, v) {
    o["default"] = v;
};
function __importStar(mod) {
    if (mod && mod.__esModule)
        return mod;
    var result = {};
    if (mod != null)
        for (var k in mod)
            if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k))
                __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
}
function __importDefault(mod) {
    return (mod && mod.__esModule) ? mod : { default: mod };
}
function __classPrivateFieldGet(receiver, state, kind, f) {
    if (kind === "a" && !f)
        throw new TypeError("Private accessor was defined without a getter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver))
        throw new TypeError("Cannot read private member from an object whose class did not declare it");
    return kind === "m" ? f : kind === "a" ? f.call(receiver) : f ? f.value : state.get(receiver);
}
function __classPrivateFieldSet(receiver, state, value, kind, f) {
    if (kind === "m")
        throw new TypeError("Private method is not writable");
    if (kind === "a" && !f)
        throw new TypeError("Private accessor was defined without a setter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver))
        throw new TypeError("Cannot write private member to an object whose class did not declare it");
    return (kind === "a" ? f.call(receiver, value) : f ? f.value = value : state.set(receiver, value)), value;
}

;// CONCATENATED MODULE: ./lib/polyfill/array.js

// https://github.com/jonathantneal/array-flat-polyfill/blob/master/src/polyfill-flat.js
Object.defineProperty(Array.prototype, 'flat', {
    configurable: true,
    value: function flat() {
        var depth = isNaN(arguments[0]) ? 1 : Number(arguments[0]);
        return depth ? Array.prototype.reduce.call(this, function (acc, cur) {
            if (Array.isArray(cur)) {
                acc.push.apply(acc, flat.call(cur, depth - 1));
            }
            else {
                acc.push(cur);
            }
            return acc;
        }, []) : Array.prototype.slice.call(this);
    },
    writable: true
});
// https://github.com/jonathantneal/array-flat-polyfill/blob/master/src/polyfill-flatMap.js
Object.defineProperty(Array.prototype, 'flatMap', {
    configurable: true,
    value: function flatMap(callback) {
        return Array.prototype.map.apply(this, arguments).flat();
    },
    writable: true
});
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/find
Array.prototype.find = function (predicate) {
    if (this === null) {
        throw new TypeError('Array.prototype.find called on null or undefined');
    }
    if (typeof predicate !== 'function') {
        throw new TypeError('predicate must be a function');
    }
    var list = Object(this);
    var length = list.length >>> 0;
    var thisArg = arguments[1];
    var value;
    for (var i = 0; i < length; i++) {
        value = list[i];
        if (predicate.call(thisArg, value, i, list)) {
            return value;
        }
    }
    return undefined;
};

// EXTERNAL MODULE: ./lib/polyfill/buffer.ts
var buffer = __webpack_require__(213);
;// CONCATENATED MODULE: ./lib/polyfill/object.js

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/entries
Object.entries = function (obj) {
    var ownProps = Object.keys(obj), i = ownProps.length, resArray = new Array(i); // preallocate the Array
    while (i--)
        resArray[i] = [ownProps[i], obj[ownProps[i]]];
    return resArray;
};

;// CONCATENATED MODULE: ./lib/polyfill/index.ts




;// CONCATENATED MODULE: ./lib/response-error.ts

var InvalidResponseError = /** @class */ (function (_super) {
    __extends(InvalidResponseError, _super);
    function InvalidResponseError() {
        var _this = _super !== null && _super.apply(this, arguments) || this;
        _this.name = _this.constructor.name;
        return _this;
    }
    return InvalidResponseError;
}(Error));


;// CONCATENATED MODULE: ./lib/response-store.ts

var CompoundResponse = /** @class */ (function () {
    function CompoundResponse() {
        this.packets = [];
        this.total = 0;
        this.firstPacketTime = new Date();
    }
    CompoundResponse.prototype.addPacket = function (packet) {
        var number = packet.number;
        if (!(Number.isInteger(number) && number >= 0)) {
            throw new InvalidResponseError('packet number is not a positive integer');
        }
        if (this.total && number >= this.total) {
            throw new InvalidResponseError('incorrect packet');
        }
        this.packets[packet.number] = packet;
    };
    CompoundResponse.prototype.gotAllPackets = function () {
        if (!this.total) {
            return false;
        }
        for (var i = 0; i < this.total; i++) {
            if (!this.packets[i]) {
                return false;
            }
        }
        return true;
    };
    return CompoundResponse;
}());

var ResponseStore = /** @class */ (function () {
    function ResponseStore(Response) {
        this.Response = Response;
        this.responses = {};
    }
    ResponseStore.prototype.addPacket = function (packet) {
        var res = this.ensureResponse(packet.reqid);
        res.addPacket(packet);
        return res;
    };
    ResponseStore.prototype.ensureResponse = function (reqid) {
        if (!(Number.isInteger(reqid) && reqid >= 0)) {
            throw new InvalidResponseError('request id is not a positive integer');
        }
        if (!this.responses[reqid]) {
            this.responses[reqid] = new this.Response();
        }
        return this.responses[reqid];
    };
    ResponseStore.prototype.remove = function (reqid) {
        delete this.responses[reqid];
    };
    return ResponseStore;
}());


;// CONCATENATED MODULE: ./protocols/quake.ts

var info = {
    id: 'quake',
    name: 'Quake',
    feature: 'query',
    transport: 'udp',
};
var parseQuakeInfo = function (data) {
    if (!data.startsWith('\\')) {
        throw new InvalidResponseError('no leading backslash');
    }
    var parts = data.substr(1).split('\\');
    var count = parts.length;
    var dict = {};
    for (var i = 0; i < count; i += 2) {
        var key = parts[i];
        var val = parts[i + 1];
        dict[key] = val;
    }
    return dict;
};

;// CONCATENATED MODULE: ./protocols/gamespy.ts





var gamespy_info = {
    id: 'gamespy',
    name: 'GameSpy',
    feature: 'query',
    transport: 'udp',
};
var GamespyResponse = /** @class */ (function (_super) {
    __extends(GamespyResponse, _super);
    function GamespyResponse() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    GamespyResponse.prototype.combine = function () {
        var infos = this.packets.map(function (packet) { return packet.data; });
        return Object.assign.apply(Object, __spreadArray([{}], infos, false));
    };
    return GamespyResponse;
}(CompoundResponse));
var GamespyResponseStore = /** @class */ (function (_super) {
    __extends(GamespyResponseStore, _super);
    function GamespyResponseStore() {
        return _super.call(this, GamespyResponse) || this;
    }
    GamespyResponseStore.prototype.addPacket = function (packet) {
        var reqid = packet.reqid, fragment = packet.number, final = packet.final;
        var res = this.ensureResponse(reqid);
        if (final) {
            if (res.total) {
                throw new InvalidResponseError('multiple final packets');
            }
            res.total = fragment + 1;
        }
        return _super.prototype.addPacket.call(this, packet);
    };
    return GamespyResponseStore;
}(ResponseStore));
var responses = new GamespyResponseStore();
var createPacket = function (values) {
    var str = Object.entries(values).map(function (_a) {
        var key = _a[0], value = _a[1];
        return "\\".concat(key, "\\").concat(value);
    }).join('');
    return Buffer.from(str);
};
var query = function () {
    return gsw.send(createPacket({
        basic: '',
        info: '',
        players: '',
    }));
};
var normalizeServerInfo = function (inf) {
    var _a;
    return (_a = {},
        _a["server-name" /* SERVER_NAME */] = String(inf.hostname),
        _a["num-players" /* NUM_PLAYERS */] = Number(inf.numplayers),
        _a["max-players" /* MAX_PLAYERS */] = Number(inf.maxplayers),
        _a["map" /* MAP */] = String(inf.maptitle || inf.mapname),
        _a["game-version" /* GAME_VERSION */] = String(inf.gamever),
        _a["game-mode" /* GAME_MODE */] = String(inf.gametype || inf.gamemode),
        _a["private" /* PRIVATE */] = inf.password === '1',
        _a);
};
var normalizePlayerList = function (inf) {
    var all_keys = Object.keys(inf);
    var players = [];
    var pfields = [{
            title: 'Name',
            kind: 'string',
            field: 'name',
            main: true,
        }];
    var _loop_1 = function (i) {
        var ending = "_".concat(i);
        var keys = all_keys.filter(function (key) { return key.endsWith(ending); });
        if (!keys.length) {
            return "break";
        }
        var player = {};
        keys.forEach(function (key) {
            var fleid = key.slice(0, -ending.length);
            player[fleid] = inf[key];
        });
        players.push(player);
    };
    for (var i = 1;; i++) {
        var state_1 = _loop_1(i);
        if (state_1 === "break")
            break;
    }
    if (players.length) {
        pfields = Object.keys(players[0]).map(function (key) { return ({
            title: key.slice(0, 1).toLocaleUpperCase() + key.slice(1),
            kind: 'string',
            field: key,
            main: key === 'name',
        }); });
    }
    return { pfields: pfields, players: players };
};
var processResponse = function (data) {
    var _a = parseQuakeInfo(data.toString()), queryid = _a.queryid, final = _a.final, info = __rest(_a, ["queryid", "final"]);
    if (!queryid) {
        throw new InvalidResponseError('no queryid found');
    }
    var _b = queryid.split('.'), reqid_str = _b[0], fragment_str = _b[1];
    var reqid = parseInt(reqid_str, 10);
    var fragment = parseInt(fragment_str, 10);
    var res = responses.addPacket({
        reqid: reqid,
        number: fragment - 1,
        final: final != null,
        data: info,
    });
    if (res.gotAllPackets()) {
        var details = res.combine();
        responses.remove(reqid);
        var inf = normalizeServerInfo(details);
        gsw.sinfo(details, inf);
        var _c = normalizePlayerList(details), pfields = _c.pfields, players = _c.players;
        gsw.plist(pfields, players);
    }
};

}();
globalThis.module = __webpack_exports__;
/******/ })()
;