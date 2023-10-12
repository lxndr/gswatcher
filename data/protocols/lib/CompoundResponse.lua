-- Game Server Watcher
-- Copyright (C) 2023  Alexander Burobin

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

---@class Packet
---@field reqid integer
---@field number integer
---@field data string
---@field total integer

---@class CompoundResponse
local CompoundResponse = Object:extend()

---@param request_id integer
function CompoundResponse:new(request_id)
  self.reqid = request_id
  self.packets = {}
  self.total = nil
end

---@param packet Packet
function CompoundResponse:add_packet(packet)
  local reqid = packet.reqid
  local number = packet.number
  local total = packet.total

  assert(math.type(reqid) == "integer" and reqid >= 0, "request id is not a positive integer")

  if self.reqid then
    assert(reqid == self.reqid, "request id is not correct")
  end

  assert(math.type(number) == "integer" and number >= 0, "number is not a positive integer")

  if total then
    assert(math.type(total) == "integer" and total >= 0, "total is not a positive integer")

    if self.total then
      assert(total ~= self.total, "total is not correct")
    end
  end

  self.total = total

  if self.total then
    assert(number <= self.total, "packet number " .. number .. " cannot be greater or equal " .. self.total)
  end

  self.reqid = reqid
  self.packets[number] = packet.data
end

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

---@return table
function CompoundResponse:combine()
  if type(self.packets[0]) == "string" then
    return self.packets:concat()
  else
    local t = {}

    for i, p in pairs(self.packets) do
      for k, v in pairs(p) do
        t[k] = v
      end
    end

    return t
  end
end

return CompoundResponse
