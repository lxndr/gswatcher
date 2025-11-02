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

---@class Packet
---@field reqid integer
---@field number integer
---@field data Buffer|Dictionary
---@field total integer

---@class (exact) CompoundResponse: Object
---@field reqid integer|nil
---@field packets (Buffer|Dictionary)[]
---@field total integer|nil
local CompoundResponse = Object:extend()

---@param request_id? integer
function CompoundResponse:new(request_id)
  self.reqid = request_id
  self.packets = {}
  self.total = nil
end

function CompoundResponse:__tostring()
  return "CompoundResponse"
end

---@param packet Packet
function CompoundResponse:add_packet(packet)
  local reqid = packet.reqid
  local number = packet.number
  local total = packet.total

  assert(math.type(reqid) == "integer" and reqid >= 0, "request id is not a positive integer")
  assert(math.type(number) == "integer" and number >= 0, "number is not a positive integer")

  if reqid ~= self.reqid then
    return
  end

  if total ~= nil then
    assert(math.type(total) == "integer" and total >= 0, "total is not a positive integer")

    if self.total ~= nil then
      assert(total == self.total, "total is not correct")
    end

    self.total = total
  end

  if self.total ~= nil then
    assert(number <= self.total, "packet number " .. number .. " cannot be greater or equal " .. self.total)
  end

  self.reqid = reqid
  self.packets[number] = packet.data
end

---@return boolean
function CompoundResponse:got_all_packets()
  if not self.total then
    return false
  end

  for i = 1, self.total do
    if not self.packets[i] then
      return false
    end
  end

  return true
end

return CompoundResponse
