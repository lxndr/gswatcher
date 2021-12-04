import JSBI from 'jsbi'

const { BigInt } = JSBI
const big32 = BigInt(32)

const readUInt64LE = (buf: Buffer, byteOffset: number) => {
  const wh = BigInt(buf.readUInt32LE(byteOffset + 4))
  const wl = BigInt(buf.readUInt32LE(byteOffset))
  return JSBI.add(JSBI.leftShift(wh, big32), wl)
}

export class DataReader {
  public pos = 0

  constructor(private buf: Buffer) {}

  skip(length: number) {
    this.pos += length
    return this
  }

  get is_end() {
    return this.pos >= this.buf.length
  }

  u8() {
    const ret = this.buf.readUInt8(this.pos)
    this.pos += 1
    return ret
  }

  u16le() {
    const ret = this.buf.readUInt16LE(this.pos)
    this.pos += 2
    return ret
  }

  u32le() {
    const ret = this.buf.readUInt32LE(this.pos)
    this.pos += 4
    return ret
  }

  u64le() {
    const ret = readUInt64LE(this.buf, this.pos)
    this.pos += 8
    return ret
  }

  i8() {
    const ret = this.buf.readInt8(this.pos)
    this.pos += 1
    return ret
  }

  i16le() {
    const ret = this.buf.readInt16LE(this.pos)
    this.pos += 2
    return ret
  }

  i32le() {
    const ret = this.buf.readInt32LE(this.pos)
    this.pos += 4
    return ret
  }

  f32le() {
    const ret = this.buf.readFloatLE(this.pos)
    this.pos += 4
    return ret
  }

  f64le() {
    const ret = this.buf.readDoubleLE(this.pos)
    this.pos += 8
    return ret
  }

  lstring(length?: number) {
    if (length == null) {
      length = this.buf.length - this.pos
    }

    const ret = this.buf.toString('binary', this.pos, this.pos + length)
    this.pos += length
    return ret
  }

  zstring(terminator = 0) {
    const idx = Array.prototype.indexOf.call(this.buf, terminator, this.pos)

    if (idx == -1) {
      return this.lstring()
    }

    const str = this.lstring(idx - this.pos)
    this.pos++
    return str
  }

  data() {
    const ret = this.buf.slice(this.pos)
    this.pos += ret.byteLength
    return ret
  }
}
