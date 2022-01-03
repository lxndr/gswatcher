export class DataWriter {
  private _buf: Buffer
  public pos = 0

  constructor() {
    this._buf = new Buffer(256)
  }

  get buf() {
    return this._buf.slice(0, this.pos)
  }

  u8(val: number) {
    this._buf.writeUInt8(val, this.pos)
    this.pos += 1
    return this
  }

  u16le(val: number) {
    this._buf.writeUInt16LE(val, this.pos)
    this.pos += 2
    return this
  }

  u32le(val: number) {
    this._buf.writeUInt32LE(val, this.pos)
    this.pos += 4
    return this
  }

  i8(val: number) {
    this._buf.writeInt8(val, this.pos)
    this.pos += 1
    return this
  }

  i16le(val: number) {
    this._buf.writeInt16LE(val, this.pos)
    this.pos += 2
    return this
  }

  i32le(val: number) {
    this._buf.writeInt32LE(val, this.pos)
    this.pos += 4
    return this
  }

  f32le(val: number) {
    this._buf.writeFloatLE(val, this.pos)
    this.pos += 4
    return this
  }

  f64le(val: number) {
    this._buf.writeDoubleLE(val, this.pos)
    this.pos += 8
    return this
  }

  zstring(val: string) {
    this.string(val, val.length)
    this.u8(0)
    return this
  }

  string(val: string, length: number = val.length) {
    this._buf.write(val, this.pos, length)
    this.pos += length
    return this
  }

  data(data: Buffer) {
    this._buf = Buffer.concat([this.buf, data])
    this.pos = this._buf.length
    return this
  }
}
