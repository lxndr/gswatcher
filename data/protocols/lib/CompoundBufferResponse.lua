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

---@class (exact) CompoundBufferResponse: CompoundResponse
---@field super CompoundResponse
---@overload fun(request_id?: integer): CompoundBufferResponse
local CompoundBufferResponse = CompoundResponse:extend()

function CompoundBufferResponse:__tostring()
  return "CompoundBufferResponse"
end

---@param packet Packet
function CompoundBufferResponse:add_packet(packet)
  assert(type(packet.data) == "string")
  CompoundBufferResponse.super.add_packet(self, packet)
end

---@return Buffer
function CompoundBufferResponse:combine()
  assert(self:got_all_packets())
  return table.concat(self.packets)
end

return CompoundBufferResponse
