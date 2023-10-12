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

local to_boolean = require("lib/to_boolean")
local DataReader = require("lib/DataReader")

---@param d table Server variables (details)
local function extract_server_info(d)
  return {
    server_name = d.hostname,
    num_players = tonumber(d.numplayers),
    max_players = tonumber(d.maxplayers),
    map = d.maptitle or d.mapname,
    game_version = d.gamever,
    game_mode = d.gametype or d.gamemode,
    private = to_boolean(d.password),
    secure = to_boolean(d.sv_punkbuster),
  }
end

---@param r DataReader
local function read_string_list(r)
  local list = {}

  while not r:is_end() do
    local s = r:zstring()

    if #s == 0 then
      break
    end

    table.insert(list, s)
  end

  return list
end

return {
  extract_server_info = extract_server_info,
  read_string_list = read_string_list,
}
