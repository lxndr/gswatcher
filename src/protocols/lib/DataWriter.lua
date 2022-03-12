local Object = require("lib/classic")

local DataWriter = Object:extend()

function DataWriter:new()
  self.buf = ""
end

function DataWriter:u8(val)
  self.buf = self.buf .. string.pack("I1", val)
  return self
end

function DataWriter:u16le(val)
  self.buf = self.buf .. string.pack("<I2", val)
  return self
end

function DataWriter:u32le(val)
  self.buf = self.buf .. string.pack("<I4", val)
  return self
end

function DataWriter:u64le(val)
  self.buf = self.buf .. string.pack("<I8", val)
  return self
end

function DataWriter:i8(val)
  self.buf = self.buf .. string.pack("i1", val)
  return self
end

function DataWriter:i16le(val)
  self.buf = self.buf .. string.pack("<i2", val)
  return self
end

function DataWriter:i32le(val)
  self.buf = self.buf .. string.pack("<i4", val)
  return self
end

function DataWriter:i64le(val)
  self.buf = self.buf .. string.pack("<i8", val)
  return self
end

function DataWriter:f32le(val)
  self.buf = self.buf .. string.pack("<f", val)
  return self
end

function DataWriter:f64le(val)
  self.buf = self.buf .. string.pack("<d", val)
  return self
end

function DataWriter:zstring(val)
  self.buf = self.buf .. string.pack("z", val)
  return self
end

function DataWriter:string(val, len)
  self.buf = self.buf .. val:sub(1, len or #val)
  return self
end

function DataWriter:data(val, len)
  self.buf = self.buf .. val:sub(1, len or #val)
  return self
end

return DataWriter
