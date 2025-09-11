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

local CompoundResponse = require("lib/CompoundResponse")

---@class (exact) CompoundDictionaryResponse: CompoundResponse
---@field super CompoundResponse
---@overload fun(request_id?: integer): CompoundDictionaryResponse
local CompoundDictionaryResponse = CompoundResponse:extend()

function CompoundDictionaryResponse:__tostring()
  return "CompoundDictionaryResponse"
end

---@param packet Packet
function CompoundDictionaryResponse:add_packet(packet)
  assert(type(packet.data) == "table")
  CompoundDictionaryResponse.super.add_packet(self, packet)
end

---@return Dictionary
function CompoundDictionaryResponse:combine()
  assert(self:got_all_packets())

  local t = {}

  for _, p in pairs(self.packets) do
    assert(type(p) == "table")

    for k, v in pairs(p) do
      t[k] = v
    end
  end

  return t
end

return CompoundDictionaryResponse
