import 'core-js/features/reflect/construct'
import jsbi from 'jsbi'

const { BigInt } = jsbi
const big32 = BigInt(32)

const readUInt64LE = (buf: Buffer, byteOffset: number) => {
  const wh = BigInt(buf.readUInt32LE(byteOffset + 4))
  const wl = BigInt(buf.readUInt32LE(byteOffset))
  return jsbi.add(jsbi.leftShift(wh, big32), wl)
}

export class DataReader {
  private _buf: Buffer
  public pos = 0

  constructor(buf: Buffer) {
    this._buf = buf
  }

  skip(length: number) {
    this.pos += length
    return this
  }

  u8() {
    const ret = this._buf.readUInt8(this.pos)
    this.pos += 1
    return ret
  }

  u16le() {
    const ret = this._buf.readUInt16LE(this.pos)
    this.pos += 2
    return ret
  }

  u32le() {
    const ret = this._buf.readUInt32LE(this.pos)
    this.pos += 4
    return ret
  }

  u64le() {
    const ret = readUInt64LE(this._buf, this.pos)
    this.pos += 8
    return ret
  }

  i8() {
    const ret = this._buf.readInt8(this.pos)
    this.pos += 1
    return ret
  }

  i16le() {
    const ret = this._buf.readInt16LE(this.pos)
    this.pos += 2
    return ret
  }

  i32le() {
    const ret = this._buf.readInt32LE(this.pos)
    this.pos += 4
    return ret
  }

  f32le() {
    const ret = this._buf.readFloatLE(this.pos)
    this.pos += 4
    return ret
  }

  f64le() {
    const ret = this._buf.readDoubleLE(this.pos)
    this.pos += 8
    return ret
  }

  zstring() {
    const idx = Array.prototype.indexOf.call(this._buf, 0, this.pos)

    if (idx == -1) {
      return this.lstring(this._buf.length - this.pos)
    }

    return this.lstring(idx - this.pos + 1)
  }

  lstring(length: number) {
    const ret = this._buf.toString('binary', this.pos, this.pos + length)
    this.pos += length
    return ret
  }

  data() {
    const ret = this._buf.slice(this.pos)
    this.pos += ret.byteLength
    return ret
  }
}
