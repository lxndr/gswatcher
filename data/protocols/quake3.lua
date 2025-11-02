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

local DataWriter = require("lib/DataWriter")
local quake2 = require("quake2")

---@type ProtocolInfo
protocol = {
  id        = "quake3",
  name      = "Quake 3",
  feature   = "query",
  transport = "udp",
}

function query()
  local w = DataWriter()
  w:i32le(-1)
  w:string("getstatus\n")
  gsw.send(w.buf)
end

---@param data Buffer
function process(data)
  quake2.process_response(data, -1, "statusResponse")
end
