export class DataWriter {
  private _buf: Uint8Array
  private _view: DataView
  public pos = 0

  constructor() {
    this._buf = new Uint8Array(256)
    this._view = new DataView(this._buf.buffer)
  }

  public get buf() {
    return this._buf.subarray(0, this.pos)
  }

  public u8(val: number) {
    this._view.setUint8(this.pos, val)
    this.pos += 1
    return this
  }

  public u16le(val: number) {
    this._view.setUint16(this.pos, val, true)
    this.pos += 2
    return this
  }

  public u32le(val: number) {
    this._view.setUint32(this.pos, val, true)
    this.pos += 4
    return this
  }

  public u64le(val: bigint) {
    this._view.setBigUint64(this.pos, val, true)
    this.pos += 8
    return this
  }

  public i8(val: number) {
    this._view.setInt8(this.pos, val)
    this.pos += 1
    return this
  }

  public i16le(val: number) {
    this._view.setInt16(this.pos, val, true)
    this.pos += 2
    return this
  }

  public i32le(val: number) {
    this._view.setInt32(this.pos, val, true)
    this.pos += 4
    return this
  }

  public i64le(val: bigint) {
    this._view.setBigInt64(this.pos, val, true)
    this.pos += 8
    return this
  }

  public f32le(val: number) {
    this._view.setFloat32(this.pos, val, true)
    this.pos += 4
    return this
  }

  public f64le(val: number) {
    this._view.setFloat64(this.pos, val, true)
    this.pos += 8
    return this
  }

  public zstring(val: string) {
    this.lstring(val, val.length)
    this.u8(0)
    return this
  }

  public lstring(val: string, length: number) {
    const encoder = new TextEncoder()
    const buf = encoder.encode(val)
    this.data(buf.subarray(0, length))
    return this
  }

  public data(data: Uint8Array) {
    this._buf.set(data, this.pos)
    this.pos += data.length
    return this
  }
}
