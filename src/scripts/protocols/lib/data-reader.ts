const getUint64BigInt = (dataview: DataView, byteOffset: number, littleEndian: boolean) => {
  const left =  dataview.getUint32(byteOffset, littleEndian);
  const right = dataview.getUint32(byteOffset + 4, littleEndian);

  return littleEndian
    ? left + 2 ** 32 * right
    : 2 ** 32 * left + right;
}

export class DataReader {
  private _view: DataView
  public pos = 0

  constructor(private _buf: Uint8Array) {
    this._view = new DataView(_buf.buffer);
  }

  public skip(length: number) {
    this.pos += length
    return this
  }

  public u8() {
    const ret = this._view.getUint8(this.pos)
    this.pos += 1
    return ret
  }

  public u16le() {
    const ret = this._view.getUint16(this.pos, true)
    this.pos += 2
    return ret
  }

  public u32le() {
    const ret = this._view.getUint32(this.pos, true)
    this.pos += 4
    return ret
  }

  public u64le() {
    const ret = getUint64BigInt(this._view, this.pos, true)
    this.pos += 8
    return ret
  }

  public i8() {
    const ret = this._view.getInt8(this.pos)
    this.pos += 1
    return ret
  }

  public i16le() {
    const ret = this._view.getInt16(this.pos, true)
    this.pos += 2
    return ret
  }

  public i32le() {
    const ret = this._view.getInt32(this.pos, true)
    this.pos += 4
    return ret
  }

  public f32le() {
    const ret = this._view.getFloat32(this.pos, true)
    this.pos += 4
    return ret
  }

  public f64le() {
    const ret = this._view.getFloat64(this.pos, true)
    this.pos += 8
    return ret
  }

  public zstring() {
    const idx = Array.prototype.indexOf.call(this._buf, 0, this.pos);

    if (idx == -1) {
      return this.lstring(this._buf.length - this.pos)
    }

    return this.lstring(idx - this.pos + 1)
  }

  public lstring(length: number) {
    const decoder = new TextDecoder('utf8')
    const offset = this._view.byteOffset + this.pos;
    const ret = decoder.decode(new DataView(this._view.buffer, offset, length))
    this.pos += length
    return ret
  }

  public data() {
    const ret = this._buf.slice(this.pos)
    this.pos += ret.byteLength
    return ret
  }
}
