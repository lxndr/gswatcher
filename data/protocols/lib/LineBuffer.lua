-- Game Server Watcher
-- Copyright (C) 2010-2026 Alexander Burobin

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

local Object = require("lib/classic")

---@class (exact) LineBuffer: Object
---@field buf string
---@overload fun(): LineBuffer
local LineBuffer = Object:extend()

function LineBuffer:new()
  self.buf = ""
end

function LineBuffer:__tostring()
  return "LineBuffer"
end

---@param data string
function LineBuffer:append(data)
  self.buf = self.buf .. data
end

function LineBuffer:clear()
  self.buf = ""
end

---@return boolean
function LineBuffer:is_empty()
  return self.buf == ""
end

---@param handler fun(line: string)
---@return integer count
function LineBuffer:consume(handler)
  local count = 0

  while true do
    local lf = self.buf:find("\n", 1, true)

    if not lf then
      break
    end

    local content_end = lf - 1
    local cr = self.buf:find("\r", 1, true)

    if cr and cr == lf - 1 then
      content_end = cr - 1
    end

    local line

    if content_end < 1 then
      line = ""
    else
      line = self.buf:sub(1, content_end)
    end

    self.buf = self.buf:sub(lf + 1)
    handler(line)
    count = count + 1
  end

  return count
end

return LineBuffer
