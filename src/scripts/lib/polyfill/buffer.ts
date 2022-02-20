Buffer.alloc = function alloc(size: number, fill?: string | Buffer | number, encoding?: BufferEncoding): Buffer {
  const buf = new Buffer(size)

  if (fill != null) {
    buf.fill(fill, 0, buf.length, encoding)
  }

  return buf
}

// @ts-expect-error
Buffer.from = function from(str: string, encoding?: BufferEncoding): Buffer {
  return new Buffer(str, encoding)
}
