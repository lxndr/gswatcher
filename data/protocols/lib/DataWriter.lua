-- Game Server Watcher
-- Copyright (C) 2025  Alexander Burobin

-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as
-- published by the Free Software Foundation, either version 3 of the
-- License, or (at your option) any later version.

-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.

-- You should have received a copy of the GNU Affero General Public License
-- along with this program.  If not, see <https://www.gnu.org/licenses/>.

---@meta

local Object = require("lib/classic")

---@class DataWriter
local DataWriter = Object:extend()

function DataWriter:new()
  self.buf = ""
end

---@param val integer
---@return DataWriter
function DataWriter:u8(val)
  self.buf = self.buf .. string.pack("I1", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:u16le(val)
  self.buf = self.buf .. string.pack("<I2", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:u16be(val)
  self.buf = self.buf .. string.pack(">I2", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:u32le(val)
  self.buf = self.buf .. string.pack("<I4", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:u32be(val)
  self.buf = self.buf .. string.pack(">I4", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:u64le(val)
  self.buf = self.buf .. string.pack("<I8", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:u64be(val)
  self.buf = self.buf .. string.pack(">I8", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i8(val)
  self.buf = self.buf .. string.pack("i1", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i16le(val)
  self.buf = self.buf .. string.pack("<i2", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i16be(val)
  self.buf = self.buf .. string.pack(">i2", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i32le(val)
  self.buf = self.buf .. string.pack("<i4", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i32be(val)
  self.buf = self.buf .. string.pack(">i4", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i64le(val)
  self.buf = self.buf .. string.pack("<i8", val)
  return self
end

---@param val integer
---@return DataWriter
function DataWriter:i64be(val)
  self.buf = self.buf .. string.pack(">i8", val)
  return self
end

---@param val number
---@return DataWriter
function DataWriter:f32le(val)
  self.buf = self.buf .. string.pack("<f", val)
  return self
end

---@param val number
---@return DataWriter
function DataWriter:f32be(val)
  self.buf = self.buf .. string.pack(">f", val)
  return self
end

---@param val number
---@return DataWriter
function DataWriter:f64le(val)
  self.buf = self.buf .. string.pack("<d", val)
  return self
end

---@param val number
---@return DataWriter
function DataWriter:f64be(val)
  self.buf = self.buf .. string.pack(">d", val)
  return self
end

---@param val string
---@return DataWriter
function DataWriter:zstring(val)
  self.buf = self.buf .. string.pack("z", val)
  return self
end

---@param val string
---@param len integer
---@return DataWriter
function DataWriter:string(val, len)
  self.buf = self.buf .. val:sub(1, len or #val)
  return self
end

---@param val string
---@param len integer
---@return DataWriter
function DataWriter:data(val, len)
  self.buf = self.buf .. val:sub(1, len or #val)
  return self
end

return DataWriter
