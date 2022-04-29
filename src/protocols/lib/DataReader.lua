local Object = require("lib/classic")

local DataReader = Object:extend()

function DataReader:new(buf)
  self.buf = buf
  self.pos = 1
end

function DataReader:is_end()
  return self.pos > #self.buf
end

function DataReader:skip(length)
  self.pos = self.pos + length
end

function DataReader:u8()
  local val, pos = string.unpack("I1", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:u16le()
  local val, pos = string.unpack("<I2", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:u32le()
  local val, pos = string.unpack("<I4", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:u64le()
  local val, pos = string.unpack("<I8", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:i8()
  local val, pos = string.unpack("i1", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:i16le()
  local val, pos = string.unpack("<i2", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:i32le()
  local val, pos = string.unpack("<i4", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:i64le()
  local val, pos = string.unpack("<i8", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:f32le()
  local val, pos = string.unpack("<f", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:f64le()
  local val, pos = string.unpack("<d", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:string(length)
  local val, pos = string.unpack("c" .. length, self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:lstring()
  local val, pos = string.unpack("s1", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:zstring()
  local val, pos = string.unpack("z", self.buf, self.pos)
  self.pos = pos
  return val
end

function DataReader:data()
  local data = self.buf:sub(self.pos)
  self.pos = #self.buf
  return data
end

return DataReader
