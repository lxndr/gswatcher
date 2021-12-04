/******/ (function() { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ 131:
/*!********************************************!*\
  !*** ./node_modules/jsbi/dist/jsbi-cjs.js ***!
  \********************************************/
/***/ (function(module) {


var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== "function" && b !== null)
            throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var JSBI = /** @class */ (function (_super) {
    __extends(JSBI, _super);
    function JSBI(i, _) {
        var _this = this;
        if (_this = _super.call(this, i) || this, _this.sign = _, Object.setPrototypeOf(_this, JSBI.prototype), i > JSBI.__kMaxLength)
            throw new RangeError("Maximum BigInt size exceeded");
        return _this;
    }
    JSBI.BigInt = function (i) { var _ = Math.floor, t = Number.isFinite; if ("number" == typeof i) {
        if (0 === i)
            return JSBI.__zero();
        if (JSBI.__isOneDigitInt(i))
            return 0 > i ? JSBI.__oneDigit(-i, !0) : JSBI.__oneDigit(i, !1);
        if (!t(i) || _(i) !== i)
            throw new RangeError("The number " + i + " cannot be converted to BigInt because it is not an integer");
        return JSBI.__fromDouble(i);
    } if ("string" == typeof i) {
        var _1 = JSBI.__fromString(i);
        if (null === _1)
            throw new SyntaxError("Cannot convert " + i + " to a BigInt");
        return _1;
    } if ("boolean" == typeof i)
        return !0 === i ? JSBI.__oneDigit(1, !1) : JSBI.__zero(); if ("object" == typeof i) {
        if (i.constructor === JSBI)
            return i;
        var _2 = JSBI.__toPrimitive(i);
        return JSBI.BigInt(_2);
    } throw new TypeError("Cannot convert " + i + " to a BigInt"); };
    JSBI.prototype.toDebugString = function () { var i = ["BigInt["]; for (var _i = 0, _a = this; _i < _a.length; _i++) {
        var _ = _a[_i];
        i.push((_ ? (_ >>> 0).toString(16) : _) + ", ");
    } return i.push("]"), i.join(""); };
    JSBI.prototype.toString = function (i) {
        if (i === void 0) { i = 10; }
        if (2 > i || 36 < i)
            throw new RangeError("toString() radix argument must be between 2 and 36");
        return 0 === this.length ? "0" : 0 == (i & i - 1) ? JSBI.__toStringBasePowerOfTwo(this, i) : JSBI.__toStringGeneric(this, i, !1);
    };
    JSBI.prototype.valueOf = function () { throw new Error("Convert JSBI instances to native numbers using `toNumber`."); };
    JSBI.toNumber = function (i) { var _ = i.length; if (0 === _)
        return 0; if (1 === _) {
        var _3 = i.__unsignedDigit(0);
        return i.sign ? -_3 : _3;
    } var t = i.__digit(_ - 1), e = JSBI.__clz30(t), n = 30 * _ - e; if (1024 < n)
        return i.sign ? -Infinity : 1 / 0; var g = n - 1, o = t, s = _ - 1; var l = e + 3; var r = 32 === l ? 0 : o << l; r >>>= 12; var a = l - 12; var u = 12 <= l ? 0 : o << 20 + l, d = 20 + l; for (0 < a && 0 < s && (s--, o = i.__digit(s), r |= o >>> 30 - a, u = o << a + 2, d = a + 2); 0 < d && 0 < s;)
        s--, o = i.__digit(s), u |= 30 <= d ? o << d - 30 : o >>> 30 - d, d -= 30; var h = JSBI.__decideRounding(i, d, s, o); if ((1 === h || 0 === h && 1 == (1 & u)) && (u = u + 1 >>> 0, 0 === u && (r++, 0 != r >>> 20 && (r = 0, g++, 1023 < g))))
        return i.sign ? -Infinity : 1 / 0; var m = i.sign ? -2147483648 : 0; return g = g + 1023 << 20, JSBI.__kBitConversionInts[1] = m | g | r, JSBI.__kBitConversionInts[0] = u, JSBI.__kBitConversionDouble[0]; };
    JSBI.unaryMinus = function (i) { if (0 === i.length)
        return i; var _ = i.__copy(); return _.sign = !i.sign, _; };
    JSBI.bitwiseNot = function (i) { return i.sign ? JSBI.__absoluteSubOne(i).__trim() : JSBI.__absoluteAddOne(i, !0); };
    JSBI.exponentiate = function (i, _) { if (_.sign)
        throw new RangeError("Exponent must be positive"); if (0 === _.length)
        return JSBI.__oneDigit(1, !1); if (0 === i.length)
        return i; if (1 === i.length && 1 === i.__digit(0))
        return i.sign && 0 == (1 & _.__digit(0)) ? JSBI.unaryMinus(i) : i; if (1 < _.length)
        throw new RangeError("BigInt too big"); var t = _.__unsignedDigit(0); if (1 === t)
        return i; if (t >= JSBI.__kMaxLengthBits)
        throw new RangeError("BigInt too big"); if (1 === i.length && 2 === i.__digit(0)) {
        var _4 = 1 + (0 | t / 30), e_1 = i.sign && 0 != (1 & t), n_1 = new JSBI(_4, e_1);
        n_1.__initializeDigits();
        var g = 1 << t % 30;
        return n_1.__setDigit(_4 - 1, g), n_1;
    } var e = null, n = i; for (0 != (1 & t) && (e = i), t >>= 1; 0 !== t; t >>= 1)
        n = JSBI.multiply(n, n), 0 != (1 & t) && (null === e ? e = n : e = JSBI.multiply(e, n)); return e; };
    JSBI.multiply = function (_, t) { if (0 === _.length)
        return _; if (0 === t.length)
        return t; var i = _.length + t.length; 30 <= _.__clzmsd() + t.__clzmsd() && i--; var e = new JSBI(i, _.sign !== t.sign); e.__initializeDigits(); for (var n = 0; n < _.length; n++)
        JSBI.__multiplyAccumulate(t, _.__digit(n), e, n); return e.__trim(); };
    JSBI.divide = function (i, _) { if (0 === _.length)
        throw new RangeError("Division by zero"); if (0 > JSBI.__absoluteCompare(i, _))
        return JSBI.__zero(); var t = i.sign !== _.sign, e = _.__unsignedDigit(0); var n; if (1 === _.length && 32767 >= e) {
        if (1 === e)
            return t === i.sign ? i : JSBI.unaryMinus(i);
        n = JSBI.__absoluteDivSmall(i, e, null);
    }
    else
        n = JSBI.__absoluteDivLarge(i, _, !0, !1); return n.sign = t, n.__trim(); };
    JSBI.remainder = function (i, _) { if (0 === _.length)
        throw new RangeError("Division by zero"); if (0 > JSBI.__absoluteCompare(i, _))
        return i; var t = _.__unsignedDigit(0); if (1 === _.length && 32767 >= t) {
        if (1 === t)
            return JSBI.__zero();
        var _5 = JSBI.__absoluteModSmall(i, t);
        return 0 === _5 ? JSBI.__zero() : JSBI.__oneDigit(_5, i.sign);
    } var e = JSBI.__absoluteDivLarge(i, _, !1, !0); return e.sign = i.sign, e.__trim(); };
    JSBI.add = function (i, _) { var t = i.sign; return t === _.sign ? JSBI.__absoluteAdd(i, _, t) : 0 <= JSBI.__absoluteCompare(i, _) ? JSBI.__absoluteSub(i, _, t) : JSBI.__absoluteSub(_, i, !t); };
    JSBI.subtract = function (i, _) { var t = i.sign; return t === _.sign ? 0 <= JSBI.__absoluteCompare(i, _) ? JSBI.__absoluteSub(i, _, t) : JSBI.__absoluteSub(_, i, !t) : JSBI.__absoluteAdd(i, _, t); };
    JSBI.leftShift = function (i, _) { return 0 === _.length || 0 === i.length ? i : _.sign ? JSBI.__rightShiftByAbsolute(i, _) : JSBI.__leftShiftByAbsolute(i, _); };
    JSBI.signedRightShift = function (i, _) { return 0 === _.length || 0 === i.length ? i : _.sign ? JSBI.__leftShiftByAbsolute(i, _) : JSBI.__rightShiftByAbsolute(i, _); };
    JSBI.unsignedRightShift = function () { throw new TypeError("BigInts have no unsigned right shift; use >> instead"); };
    JSBI.lessThan = function (i, _) { return 0 > JSBI.__compareToBigInt(i, _); };
    JSBI.lessThanOrEqual = function (i, _) { return 0 >= JSBI.__compareToBigInt(i, _); };
    JSBI.greaterThan = function (i, _) { return 0 < JSBI.__compareToBigInt(i, _); };
    JSBI.greaterThanOrEqual = function (i, _) { return 0 <= JSBI.__compareToBigInt(i, _); };
    JSBI.equal = function (_, t) { if (_.sign !== t.sign)
        return !1; if (_.length !== t.length)
        return !1; for (var e = 0; e < _.length; e++)
        if (_.__digit(e) !== t.__digit(e))
            return !1; return !0; };
    JSBI.notEqual = function (i, _) { return !JSBI.equal(i, _); };
    JSBI.bitwiseAnd = function (i, _) {
        var _a;
        var t = Math.max;
        if (!i.sign && !_.sign)
            return JSBI.__absoluteAnd(i, _).__trim();
        if (i.sign && _.sign) {
            var e = t(i.length, _.length) + 1;
            var n = JSBI.__absoluteSubOne(i, e);
            var g = JSBI.__absoluteSubOne(_);
            return n = JSBI.__absoluteOr(n, g, n), JSBI.__absoluteAddOne(n, !0, n).__trim();
        }
        return i.sign && (_a = [_, i], i = _a[0], _ = _a[1], _a), JSBI.__absoluteAndNot(i, JSBI.__absoluteSubOne(_)).__trim();
    };
    JSBI.bitwiseXor = function (i, _) {
        var _a;
        var t = Math.max;
        if (!i.sign && !_.sign)
            return JSBI.__absoluteXor(i, _).__trim();
        if (i.sign && _.sign) {
            var e_2 = t(i.length, _.length), n_2 = JSBI.__absoluteSubOne(i, e_2), g = JSBI.__absoluteSubOne(_);
            return JSBI.__absoluteXor(n_2, g, n_2).__trim();
        }
        var e = t(i.length, _.length) + 1;
        i.sign && (_a = [_, i], i = _a[0], _ = _a[1], _a);
        var n = JSBI.__absoluteSubOne(_, e);
        return n = JSBI.__absoluteXor(n, i, n), JSBI.__absoluteAddOne(n, !0, n).__trim();
    };
    JSBI.bitwiseOr = function (i, _) {
        var _a;
        var t = Math.max;
        var e = t(i.length, _.length);
        if (!i.sign && !_.sign)
            return JSBI.__absoluteOr(i, _).__trim();
        if (i.sign && _.sign) {
            var t_1 = JSBI.__absoluteSubOne(i, e);
            var n_3 = JSBI.__absoluteSubOne(_);
            return t_1 = JSBI.__absoluteAnd(t_1, n_3, t_1), JSBI.__absoluteAddOne(t_1, !0, t_1).__trim();
        }
        i.sign && (_a = [_, i], i = _a[0], _ = _a[1], _a);
        var n = JSBI.__absoluteSubOne(_, e);
        return n = JSBI.__absoluteAndNot(n, i, n), JSBI.__absoluteAddOne(n, !0, n).__trim();
    };
    JSBI.asIntN = function (_, t) { var i = Math.floor; if (0 === t.length)
        return t; if (_ = i(_), 0 > _)
        throw new RangeError("Invalid value: not (convertible to) a safe integer"); if (0 === _)
        return JSBI.__zero(); if (_ >= JSBI.__kMaxLengthBits)
        return t; var e = 0 | (_ + 29) / 30; if (t.length < e)
        return t; var g = t.__unsignedDigit(e - 1), o = 1 << (_ - 1) % 30; if (t.length === e && g < o)
        return t; if (!((g & o) === o))
        return JSBI.__truncateToNBits(_, t); if (!t.sign)
        return JSBI.__truncateAndSubFromPowerOfTwo(_, t, !0); if (0 == (g & o - 1)) {
        for (var n = e - 2; 0 <= n; n--)
            if (0 !== t.__digit(n))
                return JSBI.__truncateAndSubFromPowerOfTwo(_, t, !1);
        return t.length === e && g === o ? t : JSBI.__truncateToNBits(_, t);
    } return JSBI.__truncateAndSubFromPowerOfTwo(_, t, !1); };
    JSBI.asUintN = function (i, _) { var t = Math.floor; if (0 === _.length)
        return _; if (i = t(i), 0 > i)
        throw new RangeError("Invalid value: not (convertible to) a safe integer"); if (0 === i)
        return JSBI.__zero(); if (_.sign) {
        if (i > JSBI.__kMaxLengthBits)
            throw new RangeError("BigInt too big");
        return JSBI.__truncateAndSubFromPowerOfTwo(i, _, !1);
    } if (i >= JSBI.__kMaxLengthBits)
        return _; var e = 0 | (i + 29) / 30; if (_.length < e)
        return _; var g = i % 30; if (_.length == e) {
        if (0 === g)
            return _;
        var i_1 = _.__digit(e - 1);
        if (0 == i_1 >>> g)
            return _;
    } return JSBI.__truncateToNBits(i, _); };
    JSBI.ADD = function (i, _) { if (i = JSBI.__toPrimitive(i), _ = JSBI.__toPrimitive(_), "string" == typeof i)
        return "string" != typeof _ && (_ = _.toString()), i + _; if ("string" == typeof _)
        return i.toString() + _; if (i = JSBI.__toNumeric(i), _ = JSBI.__toNumeric(_), JSBI.__isBigInt(i) && JSBI.__isBigInt(_))
        return JSBI.add(i, _); if ("number" == typeof i && "number" == typeof _)
        return i + _; throw new TypeError("Cannot mix BigInt and other types, use explicit conversions"); };
    JSBI.LT = function (i, _) { return JSBI.__compare(i, _, 0); };
    JSBI.LE = function (i, _) { return JSBI.__compare(i, _, 1); };
    JSBI.GT = function (i, _) { return JSBI.__compare(i, _, 2); };
    JSBI.GE = function (i, _) { return JSBI.__compare(i, _, 3); };
    JSBI.EQ = function (i, _) { for (;;) {
        if (JSBI.__isBigInt(i))
            return JSBI.__isBigInt(_) ? JSBI.equal(i, _) : JSBI.EQ(_, i);
        if ("number" == typeof i) {
            if (JSBI.__isBigInt(_))
                return JSBI.__equalToNumber(_, i);
            if ("object" != typeof _)
                return i == _;
            _ = JSBI.__toPrimitive(_);
        }
        else if ("string" == typeof i) {
            if (JSBI.__isBigInt(_))
                return i = JSBI.__fromString(i), null !== i && JSBI.equal(i, _);
            if ("object" != typeof _)
                return i == _;
            _ = JSBI.__toPrimitive(_);
        }
        else if ("boolean" == typeof i) {
            if (JSBI.__isBigInt(_))
                return JSBI.__equalToNumber(_, +i);
            if ("object" != typeof _)
                return i == _;
            _ = JSBI.__toPrimitive(_);
        }
        else if ("symbol" == typeof i) {
            if (JSBI.__isBigInt(_))
                return !1;
            if ("object" != typeof _)
                return i == _;
            _ = JSBI.__toPrimitive(_);
        }
        else if ("object" == typeof i) {
            if ("object" == typeof _ && _.constructor !== JSBI)
                return i == _;
            i = JSBI.__toPrimitive(i);
        }
        else
            return i == _;
    } };
    JSBI.NE = function (i, _) { return !JSBI.EQ(i, _); };
    JSBI.__zero = function () { return new JSBI(0, !1); };
    JSBI.__oneDigit = function (i, _) { var t = new JSBI(1, _); return t.__setDigit(0, i), t; };
    JSBI.prototype.__copy = function () { var _ = new JSBI(this.length, this.sign); for (var t = 0; t < this.length; t++)
        _[t] = this[t]; return _; };
    JSBI.prototype.__trim = function () { var i = this.length, _ = this[i - 1]; for (; 0 === _;)
        i--, _ = this[i - 1], this.pop(); return 0 === i && (this.sign = !1), this; };
    JSBI.prototype.__initializeDigits = function () { for (var _ = 0; _ < this.length; _++)
        this[_] = 0; };
    JSBI.__decideRounding = function (i, _, t, e) { if (0 < _)
        return -1; var n; if (0 > _)
        n = -_ - 1;
    else {
        if (0 === t)
            return -1;
        t--, e = i.__digit(t), n = 29;
    } var g = 1 << n; if (0 == (e & g))
        return -1; if (g -= 1, 0 != (e & g))
        return 1; for (; 0 < t;)
        if (t--, 0 !== i.__digit(t))
            return 1; return 0; };
    JSBI.__fromDouble = function (i) { JSBI.__kBitConversionDouble[0] = i; var _ = 2047 & JSBI.__kBitConversionInts[1] >>> 20, t = _ - 1023, e = (0 | t / 30) + 1, n = new JSBI(e, 0 > i); var g = 1048575 & JSBI.__kBitConversionInts[1] | 1048576, o = JSBI.__kBitConversionInts[0]; var s = 20, l = t % 30; var r, a = 0; if (l < 20) {
        var i_2 = s - l;
        a = i_2 + 32, r = g >>> i_2, g = g << 32 - i_2 | o >>> i_2, o <<= 32 - i_2;
    }
    else if (l === 20)
        a = 32, r = g, g = o, o = 0;
    else {
        var i_3 = l - s;
        a = 32 - i_3, r = g << i_3 | o >>> 32 - i_3, g = o << i_3, o = 0;
    } n.__setDigit(e - 1, r); for (var _6 = e - 2; 0 <= _6; _6--)
        0 < a ? (a -= 30, r = g >>> 2, g = g << 30 | o >>> 2, o <<= 30) : r = 0, n.__setDigit(_6, r); return n.__trim(); };
    JSBI.__isWhitespace = function (i) { return !!(13 >= i && 9 <= i) || (159 >= i ? 32 == i : 131071 >= i ? 160 == i || 5760 == i : 196607 >= i ? (i &= 131071, 10 >= i || 40 == i || 41 == i || 47 == i || 95 == i || 4096 == i) : 65279 == i); };
    JSBI.__fromString = function (i, _) {
        if (_ === void 0) { _ = 0; }
        var t = 0;
        var e = i.length;
        var n = 0;
        if (n === e)
            return JSBI.__zero();
        var g = i.charCodeAt(n);
        for (; JSBI.__isWhitespace(g);) {
            if (++n === e)
                return JSBI.__zero();
            g = i.charCodeAt(n);
        }
        if (43 === g) {
            if (++n === e)
                return null;
            g = i.charCodeAt(n), t = 1;
        }
        else if (45 === g) {
            if (++n === e)
                return null;
            g = i.charCodeAt(n), t = -1;
        }
        if (0 === _) {
            if (_ = 10, 48 === g) {
                if (++n === e)
                    return JSBI.__zero();
                if (g = i.charCodeAt(n), 88 === g || 120 === g) {
                    if (_ = 16, ++n === e)
                        return null;
                    g = i.charCodeAt(n);
                }
                else if (79 === g || 111 === g) {
                    if (_ = 8, ++n === e)
                        return null;
                    g = i.charCodeAt(n);
                }
                else if (66 === g || 98 === g) {
                    if (_ = 2, ++n === e)
                        return null;
                    g = i.charCodeAt(n);
                }
            }
        }
        else if (16 === _ && 48 === g) {
            if (++n === e)
                return JSBI.__zero();
            if (g = i.charCodeAt(n), 88 === g || 120 === g) {
                if (++n === e)
                    return null;
                g = i.charCodeAt(n);
            }
        }
        if (0 != t && 10 !== _)
            return null;
        for (; 48 === g;) {
            if (++n === e)
                return JSBI.__zero();
            g = i.charCodeAt(n);
        }
        var o = e - n;
        var s = JSBI.__kMaxBitsPerChar[_], l = JSBI.__kBitsPerCharTableMultiplier - 1;
        if (o > 1073741824 / s)
            return null;
        var r = s * o + l >>> JSBI.__kBitsPerCharTableShift, a = new JSBI(0 | (r + 29) / 30, !1), u = 10 > _ ? _ : 10, h = 10 < _ ? _ - 10 : 0;
        if (0 == (_ & _ - 1)) {
            s >>= JSBI.__kBitsPerCharTableShift;
            var _7 = [], t_2 = [];
            var o_1 = !1;
            do {
                var l_1 = 0, r_1 = 0;
                for (;;) {
                    var _8 = void 0;
                    if (g - 48 >>> 0 < u)
                        _8 = g - 48;
                    else if ((32 | g) - 97 >>> 0 < h)
                        _8 = (32 | g) - 87;
                    else {
                        o_1 = !0;
                        break;
                    }
                    if (r_1 += s, l_1 = l_1 << s | _8, ++n === e) {
                        o_1 = !0;
                        break;
                    }
                    if (g = i.charCodeAt(n), 30 < r_1 + s)
                        break;
                }
                _7.push(l_1), t_2.push(r_1);
            } while (!o_1);
            JSBI.__fillFromParts(a, _7, t_2);
        }
        else {
            a.__initializeDigits();
            var t_3 = !1, o_2 = 0;
            do {
                var r_2 = 0, b = 1;
                for (;;) {
                    var s_1 = void 0;
                    if (g - 48 >>> 0 < u)
                        s_1 = g - 48;
                    else if ((32 | g) - 97 >>> 0 < h)
                        s_1 = (32 | g) - 87;
                    else {
                        t_3 = !0;
                        break;
                    }
                    var l_2 = b * _;
                    if (1073741823 < l_2)
                        break;
                    if (b = l_2, r_2 = r_2 * _ + s_1, o_2++, ++n === e) {
                        t_3 = !0;
                        break;
                    }
                    g = i.charCodeAt(n);
                }
                l = 30 * JSBI.__kBitsPerCharTableMultiplier - 1;
                var D = 0 | (s * o_2 + l >>> JSBI.__kBitsPerCharTableShift) / 30;
                a.__inplaceMultiplyAdd(b, r_2, D);
            } while (!t_3);
        }
        if (n !== e) {
            if (!JSBI.__isWhitespace(g))
                return null;
            for (n++; n < e; n++)
                if (g = i.charCodeAt(n), !JSBI.__isWhitespace(g))
                    return null;
        }
        return a.sign = -1 == t, a.__trim();
    };
    JSBI.__fillFromParts = function (_, t, e) { var n = 0, g = 0, o = 0; for (var s = t.length - 1; 0 <= s; s--) {
        var i = t[s], l = e[s];
        g |= i << o, o += l, 30 === o ? (_.__setDigit(n++, g), o = 0, g = 0) : 30 < o && (_.__setDigit(n++, 1073741823 & g), o -= 30, g = i >>> l - o);
    } if (0 !== g) {
        if (n >= _.length)
            throw new Error("implementation bug");
        _.__setDigit(n++, g);
    } for (; n < _.length; n++)
        _.__setDigit(n, 0); };
    JSBI.__toStringBasePowerOfTwo = function (_, i) { var t = _.length; var e = i - 1; e = (85 & e >>> 1) + (85 & e), e = (51 & e >>> 2) + (51 & e), e = (15 & e >>> 4) + (15 & e); var n = e, g = i - 1, o = _.__digit(t - 1), s = JSBI.__clz30(o); var l = 0 | (30 * t - s + n - 1) / n; if (_.sign && l++, 268435456 < l)
        throw new Error("string too long"); var r = Array(l); var a = l - 1, u = 0, d = 0; for (var e_3 = 0; e_3 < t - 1; e_3++) {
        var i_4 = _.__digit(e_3), t_4 = (u | i_4 << d) & g;
        r[a--] = JSBI.__kConversionChars[t_4];
        var o_3 = n - d;
        for (u = i_4 >>> o_3, d = 30 - o_3; d >= n;)
            r[a--] = JSBI.__kConversionChars[u & g], u >>>= n, d -= n;
    } var h = (u | o << d) & g; for (r[a--] = JSBI.__kConversionChars[h], u = o >>> n - d; 0 !== u;)
        r[a--] = JSBI.__kConversionChars[u & g], u >>>= n; if (_.sign && (r[a--] = "-"), -1 != a)
        throw new Error("implementation bug"); return r.join(""); };
    JSBI.__toStringGeneric = function (_, i, t) { var e = _.length; if (0 === e)
        return ""; if (1 === e) {
        var e_4 = _.__unsignedDigit(0).toString(i);
        return !1 === t && _.sign && (e_4 = "-" + e_4), e_4;
    } var n = 30 * e - JSBI.__clz30(_.__digit(e - 1)), g = JSBI.__kMaxBitsPerChar[i], o = g - 1; var s = n * JSBI.__kBitsPerCharTableMultiplier; s += o - 1, s = 0 | s / o; var l = s + 1 >> 1, r = JSBI.exponentiate(JSBI.__oneDigit(i, !1), JSBI.__oneDigit(l, !1)); var a, u; var d = r.__unsignedDigit(0); if (1 === r.length && 32767 >= d) {
        a = new JSBI(_.length, !1), a.__initializeDigits();
        var t_5 = 0;
        for (var e_5 = 2 * _.length - 1; 0 <= e_5; e_5--) {
            var i_5 = t_5 << 15 | _.__halfDigit(e_5);
            a.__setHalfDigit(e_5, 0 | i_5 / d), t_5 = 0 | i_5 % d;
        }
        u = t_5.toString(i);
    }
    else {
        var t_6 = JSBI.__absoluteDivLarge(_, r, !0, !0);
        a = t_6.quotient;
        var e_6 = t_6.remainder.__trim();
        u = JSBI.__toStringGeneric(e_6, i, !0);
    } a.__trim(); var h = JSBI.__toStringGeneric(a, i, !0); for (; u.length < l;)
        u = "0" + u; return !1 === t && _.sign && (h = "-" + h), h + u; };
    JSBI.__unequalSign = function (i) { return i ? -1 : 1; };
    JSBI.__absoluteGreater = function (i) { return i ? -1 : 1; };
    JSBI.__absoluteLess = function (i) { return i ? 1 : -1; };
    JSBI.__compareToBigInt = function (i, _) { var t = i.sign; if (t !== _.sign)
        return JSBI.__unequalSign(t); var e = JSBI.__absoluteCompare(i, _); return 0 < e ? JSBI.__absoluteGreater(t) : 0 > e ? JSBI.__absoluteLess(t) : 0; };
    JSBI.__compareToNumber = function (i, _) { if (JSBI.__isOneDigitInt(_)) {
        var t = i.sign, e = 0 > _;
        if (t !== e)
            return JSBI.__unequalSign(t);
        if (0 === i.length) {
            if (e)
                throw new Error("implementation bug");
            return 0 === _ ? 0 : -1;
        }
        if (1 < i.length)
            return JSBI.__absoluteGreater(t);
        var n = Math.abs(_), g = i.__unsignedDigit(0);
        return g > n ? JSBI.__absoluteGreater(t) : g < n ? JSBI.__absoluteLess(t) : 0;
    } return JSBI.__compareToDouble(i, _); };
    JSBI.__compareToDouble = function (i, _) { if (_ !== _)
        return _; if (_ === 1 / 0)
        return -1; if (_ === -Infinity)
        return 1; var t = i.sign; if (t !== 0 > _)
        return JSBI.__unequalSign(t); if (0 === _)
        throw new Error("implementation bug: should be handled elsewhere"); if (0 === i.length)
        return -1; JSBI.__kBitConversionDouble[0] = _; var e = 2047 & JSBI.__kBitConversionInts[1] >>> 20; if (2047 == e)
        throw new Error("implementation bug: handled elsewhere"); var n = e - 1023; if (0 > n)
        return JSBI.__absoluteGreater(t); var g = i.length; var o = i.__digit(g - 1); var s = JSBI.__clz30(o), l = 30 * g - s, r = n + 1; if (l < r)
        return JSBI.__absoluteLess(t); if (l > r)
        return JSBI.__absoluteGreater(t); var a = 1048576 | 1048575 & JSBI.__kBitConversionInts[1], u = JSBI.__kBitConversionInts[0]; var d = 20, h = 29 - s; if (h !== (0 | (l - 1) % 30))
        throw new Error("implementation bug"); var m, b = 0; if (20 > h) {
        var i_6 = d - h;
        b = i_6 + 32, m = a >>> i_6, a = a << 32 - i_6 | u >>> i_6, u <<= 32 - i_6;
    }
    else if (20 === h)
        b = 32, m = a, a = u, u = 0;
    else {
        var i_7 = h - d;
        b = 32 - i_7, m = a << i_7 | u >>> 32 - i_7, a = u << i_7, u = 0;
    } if (o >>>= 0, m >>>= 0, o > m)
        return JSBI.__absoluteGreater(t); if (o < m)
        return JSBI.__absoluteLess(t); for (var e_7 = g - 2; 0 <= e_7; e_7--) {
        0 < b ? (b -= 30, m = a >>> 2, a = a << 30 | u >>> 2, u <<= 30) : m = 0;
        var _9 = i.__unsignedDigit(e_7);
        if (_9 > m)
            return JSBI.__absoluteGreater(t);
        if (_9 < m)
            return JSBI.__absoluteLess(t);
    } if (0 !== a || 0 !== u) {
        if (0 === b)
            throw new Error("implementation bug");
        return JSBI.__absoluteLess(t);
    } return 0; };
    JSBI.__equalToNumber = function (i, _) { var t = Math.abs; return JSBI.__isOneDigitInt(_) ? 0 === _ ? 0 === i.length : 1 === i.length && i.sign === 0 > _ && i.__unsignedDigit(0) === t(_) : 0 === JSBI.__compareToDouble(i, _); };
    JSBI.__comparisonResultToBool = function (i, _) { return 0 === _ ? 0 > i : 1 === _ ? 0 >= i : 2 === _ ? 0 < i : 3 === _ ? 0 <= i : void 0; };
    JSBI.__compare = function (i, _, t) { if (i = JSBI.__toPrimitive(i), _ = JSBI.__toPrimitive(_), "string" == typeof i && "string" == typeof _)
        switch (t) {
            case 0: return i < _;
            case 1: return i <= _;
            case 2: return i > _;
            case 3: return i >= _;
        } if (JSBI.__isBigInt(i) && "string" == typeof _)
        return _ = JSBI.__fromString(_), null !== _ && JSBI.__comparisonResultToBool(JSBI.__compareToBigInt(i, _), t); if ("string" == typeof i && JSBI.__isBigInt(_))
        return i = JSBI.__fromString(i), null !== i && JSBI.__comparisonResultToBool(JSBI.__compareToBigInt(i, _), t); if (i = JSBI.__toNumeric(i), _ = JSBI.__toNumeric(_), JSBI.__isBigInt(i)) {
        if (JSBI.__isBigInt(_))
            return JSBI.__comparisonResultToBool(JSBI.__compareToBigInt(i, _), t);
        if ("number" != typeof _)
            throw new Error("implementation bug");
        return JSBI.__comparisonResultToBool(JSBI.__compareToNumber(i, _), t);
    } if ("number" != typeof i)
        throw new Error("implementation bug"); if (JSBI.__isBigInt(_))
        return JSBI.__comparisonResultToBool(JSBI.__compareToNumber(_, i), 2 ^ t); if ("number" != typeof _)
        throw new Error("implementation bug"); return 0 === t ? i < _ : 1 === t ? i <= _ : 2 === t ? i > _ : 3 === t ? i >= _ : void 0; };
    JSBI.prototype.__clzmsd = function () { return JSBI.__clz30(this.__digit(this.length - 1)); };
    JSBI.__absoluteAdd = function (_, t, e) { if (_.length < t.length)
        return JSBI.__absoluteAdd(t, _, e); if (0 === _.length)
        return _; if (0 === t.length)
        return _.sign === e ? _ : JSBI.unaryMinus(_); var n = _.length; (0 === _.__clzmsd() || t.length === _.length && 0 === t.__clzmsd()) && n++; var g = new JSBI(n, e); var o = 0, s = 0; for (; s < t.length; s++) {
        var i = _.__digit(s) + t.__digit(s) + o;
        o = i >>> 30, g.__setDigit(s, 1073741823 & i);
    } for (; s < _.length; s++) {
        var i = _.__digit(s) + o;
        o = i >>> 30, g.__setDigit(s, 1073741823 & i);
    } return s < g.length && g.__setDigit(s, o), g.__trim(); };
    JSBI.__absoluteSub = function (_, t, e) { if (0 === _.length)
        return _; if (0 === t.length)
        return _.sign === e ? _ : JSBI.unaryMinus(_); var n = new JSBI(_.length, e); var g = 0, o = 0; for (; o < t.length; o++) {
        var i = _.__digit(o) - t.__digit(o) - g;
        g = 1 & i >>> 30, n.__setDigit(o, 1073741823 & i);
    } for (; o < _.length; o++) {
        var i = _.__digit(o) - g;
        g = 1 & i >>> 30, n.__setDigit(o, 1073741823 & i);
    } return n.__trim(); };
    JSBI.__absoluteAddOne = function (_, i, t) {
        if (t === void 0) { t = null; }
        var e = _.length;
        null === t ? t = new JSBI(e, i) : t.sign = i;
        var n = 1;
        for (var g = 0; g < e; g++) {
            var i_8 = _.__digit(g) + n;
            n = i_8 >>> 30, t.__setDigit(g, 1073741823 & i_8);
        }
        return 0 != n && t.__setDigitGrow(e, 1), t;
    };
    JSBI.__absoluteSubOne = function (_, t) { var e = _.length; t = t || e; var n = new JSBI(t, !1); var g = 1; for (var o = 0; o < e; o++) {
        var i = _.__digit(o) - g;
        g = 1 & i >>> 30, n.__setDigit(o, 1073741823 & i);
    } if (0 != g)
        throw new Error("implementation bug"); for (var g_1 = e; g_1 < t; g_1++)
        n.__setDigit(g_1, 0); return n; };
    JSBI.__absoluteAnd = function (_, t, e) {
        if (e === void 0) { e = null; }
        var n = _.length, g = t.length, o = g;
        if (n < g) {
            o = n;
            var i = _, e_8 = n;
            _ = t, n = g, t = i, g = e_8;
        }
        var s = o;
        null === e ? e = new JSBI(s, !1) : s = e.length;
        var l = 0;
        for (; l < o; l++)
            e.__setDigit(l, _.__digit(l) & t.__digit(l));
        for (; l < s; l++)
            e.__setDigit(l, 0);
        return e;
    };
    JSBI.__absoluteAndNot = function (_, t, e) {
        if (e === void 0) { e = null; }
        var n = _.length, g = t.length;
        var o = g;
        n < g && (o = n);
        var s = n;
        null === e ? e = new JSBI(s, !1) : s = e.length;
        var l = 0;
        for (; l < o; l++)
            e.__setDigit(l, _.__digit(l) & ~t.__digit(l));
        for (; l < n; l++)
            e.__setDigit(l, _.__digit(l));
        for (; l < s; l++)
            e.__setDigit(l, 0);
        return e;
    };
    JSBI.__absoluteOr = function (_, t, e) {
        if (e === void 0) { e = null; }
        var n = _.length, g = t.length, o = g;
        if (n < g) {
            o = n;
            var i = _, e_9 = n;
            _ = t, n = g, t = i, g = e_9;
        }
        var s = n;
        null === e ? e = new JSBI(s, !1) : s = e.length;
        var l = 0;
        for (; l < o; l++)
            e.__setDigit(l, _.__digit(l) | t.__digit(l));
        for (; l < n; l++)
            e.__setDigit(l, _.__digit(l));
        for (; l < s; l++)
            e.__setDigit(l, 0);
        return e;
    };
    JSBI.__absoluteXor = function (_, t, e) {
        if (e === void 0) { e = null; }
        var n = _.length, g = t.length, o = g;
        if (n < g) {
            o = n;
            var i = _, e_10 = n;
            _ = t, n = g, t = i, g = e_10;
        }
        var s = n;
        null === e ? e = new JSBI(s, !1) : s = e.length;
        var l = 0;
        for (; l < o; l++)
            e.__setDigit(l, _.__digit(l) ^ t.__digit(l));
        for (; l < n; l++)
            e.__setDigit(l, _.__digit(l));
        for (; l < s; l++)
            e.__setDigit(l, 0);
        return e;
    };
    JSBI.__absoluteCompare = function (_, t) { var e = _.length - t.length; if (0 != e)
        return e; var n = _.length - 1; for (; 0 <= n && _.__digit(n) === t.__digit(n);)
        n--; return 0 > n ? 0 : _.__unsignedDigit(n) > t.__unsignedDigit(n) ? 1 : -1; };
    JSBI.__multiplyAccumulate = function (_, t, e, n) { if (0 === t)
        return; var g = 32767 & t, o = t >>> 15; var s = 0, l = 0; for (var r = void 0, a = 0; a < _.length; a++, n++) {
        r = e.__digit(n);
        var i = _.__digit(a), t_7 = 32767 & i, u = i >>> 15, d = JSBI.__imul(t_7, g), h = JSBI.__imul(t_7, o), m = JSBI.__imul(u, g), b = JSBI.__imul(u, o);
        r += l + d + s, s = r >>> 30, r &= 1073741823, r += ((32767 & h) << 15) + ((32767 & m) << 15), s += r >>> 30, l = b + (h >>> 15) + (m >>> 15), e.__setDigit(n, 1073741823 & r);
    } for (; 0 != s || 0 !== l; n++) {
        var i = e.__digit(n);
        i += s + l, l = 0, s = i >>> 30, e.__setDigit(n, 1073741823 & i);
    } };
    JSBI.__internalMultiplyAdd = function (_, t, e, g, o) { var s = e, l = 0; for (var n = 0; n < g; n++) {
        var i = _.__digit(n), e_11 = JSBI.__imul(32767 & i, t), g_2 = JSBI.__imul(i >>> 15, t), a = e_11 + ((32767 & g_2) << 15) + l + s;
        s = a >>> 30, l = g_2 >>> 15, o.__setDigit(n, 1073741823 & a);
    } if (o.length > g)
        for (o.__setDigit(g++, s + l); g < o.length;)
            o.__setDigit(g++, 0);
    else if (0 !== s + l)
        throw new Error("implementation bug"); };
    JSBI.prototype.__inplaceMultiplyAdd = function (i, _, t) { t > this.length && (t = this.length); var e = 32767 & i, n = i >>> 15; var g = 0, o = _; for (var s = 0; s < t; s++) {
        var i_9 = this.__digit(s), _10 = 32767 & i_9, t_8 = i_9 >>> 15, l = JSBI.__imul(_10, e), r = JSBI.__imul(_10, n), a = JSBI.__imul(t_8, e), u = JSBI.__imul(t_8, n);
        var d = o + l + g;
        g = d >>> 30, d &= 1073741823, d += ((32767 & r) << 15) + ((32767 & a) << 15), g += d >>> 30, o = u + (r >>> 15) + (a >>> 15), this.__setDigit(s, 1073741823 & d);
    } if (0 != g || 0 !== o)
        throw new Error("implementation bug"); };
    JSBI.__absoluteDivSmall = function (_, t, e) {
        if (e === void 0) { e = null; }
        null === e && (e = new JSBI(_.length, !1));
        var n = 0;
        for (var g = void 0, o = 2 * _.length - 1; 0 <= o; o -= 2) {
            g = (n << 15 | _.__halfDigit(o)) >>> 0;
            var i = 0 | g / t;
            n = 0 | g % t, g = (n << 15 | _.__halfDigit(o - 1)) >>> 0;
            var s = 0 | g / t;
            n = 0 | g % t, e.__setDigit(o >>> 1, i << 15 | s);
        }
        return e;
    };
    JSBI.__absoluteModSmall = function (_, t) { var e = 0; for (var n = 2 * _.length - 1; 0 <= n; n--) {
        var i = (e << 15 | _.__halfDigit(n)) >>> 0;
        e = 0 | i % t;
    } return e; };
    JSBI.__absoluteDivLarge = function (i, _, t, e) { var g = _.__halfDigitLength(), n = _.length, o = i.__halfDigitLength() - g; var s = null; t && (s = new JSBI(o + 2 >>> 1, !1), s.__initializeDigits()); var l = new JSBI(g + 2 >>> 1, !1); l.__initializeDigits(); var r = JSBI.__clz15(_.__halfDigit(g - 1)); 0 < r && (_ = JSBI.__specialLeftShift(_, r, 0)); var a = JSBI.__specialLeftShift(i, r, 1), u = _.__halfDigit(g - 1); var d = 0; for (var r_3, h = o; 0 <= h; h--) {
        r_3 = 32767;
        var i_10 = a.__halfDigit(h + g);
        if (i_10 !== u) {
            var t_9 = (i_10 << 15 | a.__halfDigit(h + g - 1)) >>> 0;
            r_3 = 0 | t_9 / u;
            var e_12 = 0 | t_9 % u;
            var n_4 = _.__halfDigit(g - 2), o_4 = a.__halfDigit(h + g - 2);
            for (; JSBI.__imul(r_3, n_4) >>> 0 > (e_12 << 16 | o_4) >>> 0 && (r_3--, e_12 += u, !(32767 < e_12));)
                ;
        }
        JSBI.__internalMultiplyAdd(_, r_3, 0, n, l);
        var e_13 = a.__inplaceSub(l, h, g + 1);
        0 !== e_13 && (e_13 = a.__inplaceAdd(_, h, g), a.__setHalfDigit(h + g, 32767 & a.__halfDigit(h + g) + e_13), r_3--), t && (1 & h ? d = r_3 << 15 : s.__setDigit(h >>> 1, d | r_3));
    } if (e)
        return a.__inplaceRightShift(r), t ? { quotient: s, remainder: a } : a; if (t)
        return s; throw new Error("unreachable"); };
    JSBI.__clz15 = function (i) { return JSBI.__clz30(i) - 15; };
    JSBI.prototype.__inplaceAdd = function (_, t, e) { var n = 0; for (var g = 0; g < e; g++) {
        var i = this.__halfDigit(t + g) + _.__halfDigit(g) + n;
        n = i >>> 15, this.__setHalfDigit(t + g, 32767 & i);
    } return n; };
    JSBI.prototype.__inplaceSub = function (_, t, e) { var n = 0; if (1 & t) {
        t >>= 1;
        var g = this.__digit(t), o = 32767 & g, s = 0;
        for (; s < e - 1 >>> 1; s++) {
            var i_11 = _.__digit(s), e_14 = (g >>> 15) - (32767 & i_11) - n;
            n = 1 & e_14 >>> 15, this.__setDigit(t + s, (32767 & e_14) << 15 | 32767 & o), g = this.__digit(t + s + 1), o = (32767 & g) - (i_11 >>> 15) - n, n = 1 & o >>> 15;
        }
        var i = _.__digit(s), l = (g >>> 15) - (32767 & i) - n;
        n = 1 & l >>> 15, this.__setDigit(t + s, (32767 & l) << 15 | 32767 & o);
        if (t + s + 1 >= this.length)
            throw new RangeError("out of bounds");
        0 == (1 & e) && (g = this.__digit(t + s + 1), o = (32767 & g) - (i >>> 15) - n, n = 1 & o >>> 15, this.__setDigit(t + _.length, 1073709056 & g | 32767 & o));
    }
    else {
        t >>= 1;
        var g = 0;
        for (; g < _.length - 1; g++) {
            var i_12 = this.__digit(t + g), e_15 = _.__digit(g), o_5 = (32767 & i_12) - (32767 & e_15) - n;
            n = 1 & o_5 >>> 15;
            var s_2 = (i_12 >>> 15) - (e_15 >>> 15) - n;
            n = 1 & s_2 >>> 15, this.__setDigit(t + g, (32767 & s_2) << 15 | 32767 & o_5);
        }
        var i = this.__digit(t + g), o = _.__digit(g), s = (32767 & i) - (32767 & o) - n;
        n = 1 & s >>> 15;
        var l = 0;
        0 == (1 & e) && (l = (i >>> 15) - (o >>> 15) - n, n = 1 & l >>> 15), this.__setDigit(t + g, (32767 & l) << 15 | 32767 & s);
    } return n; };
    JSBI.prototype.__inplaceRightShift = function (_) { if (0 === _)
        return; var t = this.__digit(0) >>> _; var e = this.length - 1; for (var n = 0; n < e; n++) {
        var i = this.__digit(n + 1);
        this.__setDigit(n, 1073741823 & i << 30 - _ | t), t = i >>> _;
    } this.__setDigit(e, t); };
    JSBI.__specialLeftShift = function (_, t, e) { var g = _.length, n = new JSBI(g + e, !1); if (0 === t) {
        for (var t_10 = 0; t_10 < g; t_10++)
            n.__setDigit(t_10, _.__digit(t_10));
        return 0 < e && n.__setDigit(g, 0), n;
    } var o = 0; for (var s = 0; s < g; s++) {
        var i = _.__digit(s);
        n.__setDigit(s, 1073741823 & i << t | o), o = i >>> 30 - t;
    } return 0 < e && n.__setDigit(g, o), n; };
    JSBI.__leftShiftByAbsolute = function (_, i) { var t = JSBI.__toShiftAmount(i); if (0 > t)
        throw new RangeError("BigInt too big"); var e = 0 | t / 30, n = t % 30, g = _.length, o = 0 !== n && 0 != _.__digit(g - 1) >>> 30 - n, s = g + e + (o ? 1 : 0), l = new JSBI(s, _.sign); if (0 === n) {
        var t_11 = 0;
        for (; t_11 < e; t_11++)
            l.__setDigit(t_11, 0);
        for (; t_11 < s; t_11++)
            l.__setDigit(t_11, _.__digit(t_11 - e));
    }
    else {
        var t_12 = 0;
        for (var _11 = 0; _11 < e; _11++)
            l.__setDigit(_11, 0);
        for (var o_6 = 0; o_6 < g; o_6++) {
            var i_13 = _.__digit(o_6);
            l.__setDigit(o_6 + e, 1073741823 & i_13 << n | t_12), t_12 = i_13 >>> 30 - n;
        }
        if (o)
            l.__setDigit(g + e, t_12);
        else if (0 !== t_12)
            throw new Error("implementation bug");
    } return l.__trim(); };
    JSBI.__rightShiftByAbsolute = function (_, i) { var t = _.length, e = _.sign, n = JSBI.__toShiftAmount(i); if (0 > n)
        return JSBI.__rightShiftByMaximum(e); var g = 0 | n / 30, o = n % 30; var s = t - g; if (0 >= s)
        return JSBI.__rightShiftByMaximum(e); var l = !1; if (e) {
        if (0 != (_.__digit(g) & (1 << o) - 1))
            l = !0;
        else
            for (var t_13 = 0; t_13 < g; t_13++)
                if (0 !== _.__digit(t_13)) {
                    l = !0;
                    break;
                }
    } if (l && 0 === o) {
        var i_14 = _.__digit(t - 1);
        0 == ~i_14 && s++;
    } var r = new JSBI(s, e); if (0 === o) {
        r.__setDigit(s - 1, 0);
        for (var e_16 = g; e_16 < t; e_16++)
            r.__setDigit(e_16 - g, _.__digit(e_16));
    }
    else {
        var e_17 = _.__digit(g) >>> o;
        var n_5 = t - g - 1;
        for (var t_14 = 0; t_14 < n_5; t_14++) {
            var i_15 = _.__digit(t_14 + g + 1);
            r.__setDigit(t_14, 1073741823 & i_15 << 30 - o | e_17), e_17 = i_15 >>> o;
        }
        r.__setDigit(n_5, e_17);
    } return l && (r = JSBI.__absoluteAddOne(r, !0, r)), r.__trim(); };
    JSBI.__rightShiftByMaximum = function (i) { return i ? JSBI.__oneDigit(1, !0) : JSBI.__zero(); };
    JSBI.__toShiftAmount = function (i) { if (1 < i.length)
        return -1; var _ = i.__unsignedDigit(0); return _ > JSBI.__kMaxLengthBits ? -1 : _; };
    JSBI.__toPrimitive = function (i, _) {
        if (_ === void 0) { _ = "default"; }
        if ("object" != typeof i)
            return i;
        if (i.constructor === JSBI)
            return i;
        if ("undefined" != typeof Symbol && "symbol" == typeof Symbol.toPrimitive) {
            var t_15 = i[Symbol.toPrimitive];
            if (t_15) {
                var i_16 = t_15(_);
                if ("object" != typeof i_16)
                    return i_16;
                throw new TypeError("Cannot convert object to primitive value");
            }
        }
        var t = i.valueOf;
        if (t) {
            var _12 = t.call(i);
            if ("object" != typeof _12)
                return _12;
        }
        var e = i.toString;
        if (e) {
            var _13 = e.call(i);
            if ("object" != typeof _13)
                return _13;
        }
        throw new TypeError("Cannot convert object to primitive value");
    };
    JSBI.__toNumeric = function (i) { return JSBI.__isBigInt(i) ? i : +i; };
    JSBI.__isBigInt = function (i) { return "object" == typeof i && null !== i && i.constructor === JSBI; };
    JSBI.__truncateToNBits = function (i, _) { var t = 0 | (i + 29) / 30, e = new JSBI(t, _.sign), n = t - 1; for (var t_16 = 0; t_16 < n; t_16++)
        e.__setDigit(t_16, _.__digit(t_16)); var g = _.__digit(n); if (0 != i % 30) {
        var _14 = 32 - i % 30;
        g = g << _14 >>> _14;
    } return e.__setDigit(n, g), e.__trim(); };
    JSBI.__truncateAndSubFromPowerOfTwo = function (_, t, e) { var n = Math.min; var g = 0 | (_ + 29) / 30, o = new JSBI(g, e); var s = 0; var l = g - 1; var a = 0; for (var i = n(l, t.length); s < i; s++) {
        var i_17 = 0 - t.__digit(s) - a;
        a = 1 & i_17 >>> 30, o.__setDigit(s, 1073741823 & i_17);
    } for (; s < l; s++)
        o.__setDigit(s, 0 | 1073741823 & -a); var u = l < t.length ? t.__digit(l) : 0; var d = _ % 30; var h; if (0 == d)
        h = 0 - u - a, h &= 1073741823;
    else {
        var i = 32 - d;
        u = u << i >>> i;
        var _15 = 1 << 32 - i;
        h = _15 - u - a, h &= _15 - 1;
    } return o.__setDigit(l, h), o.__trim(); };
    JSBI.prototype.__digit = function (_) { return this[_]; };
    JSBI.prototype.__unsignedDigit = function (_) { return this[_] >>> 0; };
    JSBI.prototype.__setDigit = function (_, i) { this[_] = 0 | i; };
    JSBI.prototype.__setDigitGrow = function (_, i) { this[_] = 0 | i; };
    JSBI.prototype.__halfDigitLength = function () { var i = this.length; return 32767 >= this.__unsignedDigit(i - 1) ? 2 * i - 1 : 2 * i; };
    JSBI.prototype.__halfDigit = function (_) { return 32767 & this[_ >>> 1] >>> 15 * (1 & _); };
    JSBI.prototype.__setHalfDigit = function (_, i) { var t = _ >>> 1, e = this.__digit(t), n = 1 & _ ? 32767 & e | i << 15 : 1073709056 & e | 32767 & i; this.__setDigit(t, n); };
    JSBI.__digitPow = function (i, _) { var t = 1; for (; 0 < _;)
        1 & _ && (t *= i), _ >>>= 1, i *= i; return t; };
    JSBI.__isOneDigitInt = function (i) { return (1073741823 & i) === i; };
    return JSBI;
}(Array));
JSBI.__kMaxLength = 33554432, JSBI.__kMaxLengthBits = JSBI.__kMaxLength << 5, JSBI.__kMaxBitsPerChar = [0, 0, 32, 51, 64, 75, 83, 90, 96, 102, 107, 111, 115, 119, 122, 126, 128, 131, 134, 136, 139, 141, 143, 145, 147, 149, 151, 153, 154, 156, 158, 159, 160, 162, 163, 165, 166], JSBI.__kBitsPerCharTableShift = 5, JSBI.__kBitsPerCharTableMultiplier = 1 << JSBI.__kBitsPerCharTableShift, JSBI.__kConversionChars = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"], JSBI.__kBitConversionBuffer = new ArrayBuffer(8), JSBI.__kBitConversionDouble = new Float64Array(JSBI.__kBitConversionBuffer), JSBI.__kBitConversionInts = new Int32Array(JSBI.__kBitConversionBuffer), JSBI.__clz30 = Math.clz32 ? function (i) { return Math.clz32(i) - 2; } : function (i) { return 0 === i ? 30 : 0 | 29 - (0 | Math.log(i >>> 0) / Math.LN2); }, JSBI.__imul = Math.imul || function (i, _) { return 0 | i * _; }, module.exports = JSBI;


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
/******/ 		__webpack_modules__[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/compat get default export */
/******/ 	!function() {
/******/ 		// getDefaultExport function for compatibility with non-harmony modules
/******/ 		__webpack_require__.n = function(module) {
/******/ 			var getter = module && module.__esModule ?
/******/ 				function() { return module['default']; } :
/******/ 				function() { return module; };
/******/ 			__webpack_require__.d(getter, { a: getter });
/******/ 			return getter;
/******/ 		};
/******/ 	}();
/******/ 	
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
/*!**************************************!*\
  !*** ./protocols/ase.ts + 4 modules ***!
  \**************************************/
// ESM COMPAT FLAG
__webpack_require__.r(__webpack_exports__);

// EXPORTS
__webpack_require__.d(__webpack_exports__, {
  "info": function() { return /* binding */ info; },
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

// EXTERNAL MODULE: ./node_modules/jsbi/dist/jsbi-cjs.js
var jsbi_cjs = __webpack_require__(131);
var jsbi_cjs_default = /*#__PURE__*/__webpack_require__.n(jsbi_cjs);
;// CONCATENATED MODULE: ./protocols/lib/data-reader.ts

var BigInt = (jsbi_cjs_default()).BigInt;
var big32 = BigInt(32);
var readUInt64LE = function (buf, byteOffset) {
    var wh = BigInt(buf.readUInt32LE(byteOffset + 4));
    var wl = BigInt(buf.readUInt32LE(byteOffset));
    return jsbi_cjs_default().add(jsbi_cjs_default().leftShift(wh, big32), wl);
};
var DataReader = /** @class */ (function () {
    function DataReader(buf) {
        this.buf = buf;
        this.pos = 0;
    }
    DataReader.prototype.skip = function (length) {
        this.pos += length;
        return this;
    };
    Object.defineProperty(DataReader.prototype, "is_end", {
        get: function () {
            return this.pos >= this.buf.length;
        },
        enumerable: false,
        configurable: true
    });
    DataReader.prototype.u8 = function () {
        var ret = this.buf.readUInt8(this.pos);
        this.pos += 1;
        return ret;
    };
    DataReader.prototype.u16le = function () {
        var ret = this.buf.readUInt16LE(this.pos);
        this.pos += 2;
        return ret;
    };
    DataReader.prototype.u32le = function () {
        var ret = this.buf.readUInt32LE(this.pos);
        this.pos += 4;
        return ret;
    };
    DataReader.prototype.u64le = function () {
        var ret = readUInt64LE(this.buf, this.pos);
        this.pos += 8;
        return ret;
    };
    DataReader.prototype.i8 = function () {
        var ret = this.buf.readInt8(this.pos);
        this.pos += 1;
        return ret;
    };
    DataReader.prototype.i16le = function () {
        var ret = this.buf.readInt16LE(this.pos);
        this.pos += 2;
        return ret;
    };
    DataReader.prototype.i32le = function () {
        var ret = this.buf.readInt32LE(this.pos);
        this.pos += 4;
        return ret;
    };
    DataReader.prototype.f32le = function () {
        var ret = this.buf.readFloatLE(this.pos);
        this.pos += 4;
        return ret;
    };
    DataReader.prototype.f64le = function () {
        var ret = this.buf.readDoubleLE(this.pos);
        this.pos += 8;
        return ret;
    };
    DataReader.prototype.lstring = function (length) {
        if (length == null) {
            length = this.buf.length - this.pos;
        }
        var ret = this.buf.toString('binary', this.pos, this.pos + length);
        this.pos += length;
        return ret;
    };
    DataReader.prototype.zstring = function (terminator) {
        if (terminator === void 0) { terminator = 0; }
        var idx = Array.prototype.indexOf.call(this.buf, terminator, this.pos);
        if (idx == -1) {
            return this.lstring();
        }
        var str = this.lstring(idx - this.pos);
        this.pos++;
        return str;
    };
    DataReader.prototype.data = function () {
        var ret = this.buf.slice(this.pos);
        this.pos += ret.byteLength;
        return ret;
    };
    return DataReader;
}());


;// CONCATENATED MODULE: ./protocols/lib/response-error.ts

var InvalidResponseError = /** @class */ (function (_super) {
    __extends(InvalidResponseError, _super);
    function InvalidResponseError() {
        var _this = _super !== null && _super.apply(this, arguments) || this;
        _this.name = _this.constructor.name;
        return _this;
    }
    return InvalidResponseError;
}(Error));


;// CONCATENATED MODULE: ./protocols/lib/str2bool.ts
var true_strings = [
    '1',
    'true',
    'on',
    'yes',
];
var str2bool = function (str) {
    return true_strings.indexOf(str.toLowerCase()) !== -1;
};

;// CONCATENATED MODULE: ./protocols/ase.ts




var info = {
    id: 'ase',
    name: 'All-Seeing Eye',
    transport: 'udp',
};
var query = function () {
    return gsw.send(new Buffer('s'));
};
var readString = function (r) {
    var len = r.u8();
    if (len == 0) {
        throw new InvalidResponseError('string must exist');
    }
    return r.lstring(len - 1);
};
var readGeneralInfo = function (r) { return ({
    gameName: readString(r),
    port: readString(r),
    serverName: readString(r),
    gameType: readString(r),
    map: readString(r),
    version: readString(r),
    private: readString(r),
    numPlayers: readString(r),
    maxPlayers: readString(r),
}); };
var readKeyValues = function (r) {
    var inf = {};
    while (!r.is_end) {
        var key = readString(r);
        if (!key) {
            break;
        }
        var val = readString(r);
        inf[key] = val;
    }
    return inf;
};
var readPlayerList = function (r) {
    var players = [];
    while (!r.is_end) {
        var player = {};
        var flags = r.u8();
        if (flags & 0x01) {
            player.name = readString(r);
        }
        if (flags & 0x02) {
            player.team = readString(r);
        }
        if (flags & 0x04) {
            player.skin = readString(r);
        }
        if (flags & 0x08) {
            player.score = readString(r);
        }
        if (flags & 0x10) {
            player.ping = readString(r);
        }
        if (flags & 0x20) {
            player.time = readString(r);
        }
        players.push(player);
    }
    return players;
};
var normalizeServerInfo = function (inf) {
    var _a;
    return (_a = {},
        _a["server-name" /* SERVER_NAME */] = inf.serverName,
        _a["game-version" /* GAME_VERSION */] = inf.version,
        _a["game-mode" /* GAME_MODE */] = inf.gameType,
        _a["map" /* MAP */] = inf.map,
        _a["private" /* PRIVATE */] = str2bool(inf.private),
        _a["num-players" /* NUM_PLAYERS */] = Number(inf.numPlayers),
        _a["max-players" /* MAX_PLAYERS */] = Number(inf.maxPlayers),
        _a);
};
var processResponse = function (data) {
    var r = new DataReader(data);
    var sig = r.lstring(4);
    if (sig !== 'EYE1') {
        throw new InvalidResponseError('not an All-Seeing Eye response');
    }
    var generalInfo = readGeneralInfo(r);
    var keyValues = readKeyValues(r);
    var players = readPlayerList(r);
    var all_info = __assign(__assign({}, generalInfo), keyValues);
    var inf = normalizeServerInfo(all_info);
    gsw.details(all_info);
    gsw.sinfo(inf);
    gsw.plist(players);
};

}();
globalThis.module = __webpack_exports__;
/******/ })()
;