---@meta

local Object = require("lib/classic")

---@class DataReader
local DataReader = Object:extend()

---@param buf string
function DataReader:new(buf)
  self.buf = buf
  self.pos = 1
end

function DataReader:is_end()
  return self.pos > #self.buf
end

---@param length integer
function DataReader:skip(length)
  self.pos = self.pos + length
end

---@return integer
function DataReader:u8()
  local val, pos = string.unpack("I1", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:u16le()
  local val, pos = string.unpack("<I2", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:u16be()
  local val, pos = string.unpack(">I2", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:u32le()
  local val, pos = string.unpack("<I4", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:u32be()
  local val, pos = string.unpack(">I4", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:u64le()
  local val, pos = string.unpack("<I8", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:u64be()
  local val, pos = string.unpack(">I8", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i8()
  local val, pos = string.unpack("i1", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i16le()
  local val, pos = string.unpack("<i2", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i16be()
  local val, pos = string.unpack(">i2", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i32le()
  local val, pos = string.unpack("<i4", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i32be()
  local val, pos = string.unpack(">i4", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i64le()
  local val, pos = string.unpack("<i8", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return integer
function DataReader:i64be()
  local val, pos = string.unpack(">i8", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return number
function DataReader:f32le()
  local val, pos = string.unpack("<f", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return number
function DataReader:f32be()
  local val, pos = string.unpack(">f", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return number
function DataReader:f64le()
  local val, pos = string.unpack("<d", self.buf, self.pos)
  self.pos = pos
  return val
end

---@return number
function DataReader:f64be()
  local val, pos = string.unpack(">d", self.buf, self.pos)
  self.pos = pos
  return val
end

---@param length integer
---@return string
function DataReader:string(length)
  local val, pos = string.unpack("c" .. length, self.buf, self.pos)
  self.pos = pos
  return val
end

---@return string
function DataReader:lstring()
  local val, pos = string.unpack("s1", self.buf, self.pos)
  self.pos = pos
  return val
end

---@param delimiter? string
---@return string
function DataReader:zstring(delimiter)
  delimiter = delimiter or "\0"
  local pos = self.buf:find(delimiter, self.pos, true)

  if pos == nil then
    pos = #self.buf + 1
  end

  local val = self.buf:sub(self.pos, pos - #delimiter)
  self.pos = pos + #delimiter
  return val
end

---@return string
function DataReader:data()
  local data = self.buf:sub(self.pos)
  self.pos = #self.buf
  return data
end

return DataReader
