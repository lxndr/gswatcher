// https://github.com/jonathantneal/array-flat-polyfill/blob/master/src/polyfill-flat.js
Object.defineProperty(Array.prototype, 'flat', {
  configurable: true,
  value: function flat () {
    var depth = isNaN(arguments[0]) ? 1 : Number(arguments[0]);

    return depth ? Array.prototype.reduce.call(this, function (acc, cur) {
      if (Array.isArray(cur)) {
        acc.push.apply(acc, flat.call(cur, depth - 1));
      } else {
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
  value: function flatMap (callback) {
    return Array.prototype.map.apply(this, arguments).flat();
  },
  writable: true
});

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/find
Array.prototype.find = function(predicate) {
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
