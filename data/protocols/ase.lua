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

local DataReader = require "lib/DataReader"
local concat_tables = require "lib/concat_tables"

protocol = {
  id = "ase",
  name = "All-Seeing Eye",
  feature = "query",
  transport = "udp",
}

local info_map = {
  game_name    = "game",
  game_version = "version",
  game_mode    = "game_type",
  server_name  = "name",
  map          = "map",
  num_players  = "num_players",
  max_players  = "max_players",
  private      = "private",
}

function query()
  gsw.send("s")
end

local function read_general_info(r)
  return {
    game_name = r:lstring(),
    port = r:lstring(),
    server_name = r:lstring(),
    game_type = r:lstring(),
    map = r:lstring(),
    version = r:lstring(),
    private = r:lstring(),
    num_players = r:lstring(),
    max_players = r:lstring(),
  }
end

local function read_key_values(r)
  local info = {}

  while not r:is_end() do
    local key = r:lstring()

    if key == "" then
      break
    end

    info[key] = r:lstring()
  end

  return info
end

local function read_player_list(r)
  local players = {}

  while not r:is_end() do
    local player = {}
    local flags = r:u8()

    if (flags & 0x01) > 0 then
      player.name = r:lstring()
    end

    if (flags & 0x02) > 0 then
      player.team = r:lstring()
    end

    if (flags & 0x04) > 0 then
      player.skin = r:lstring()
    end

    if (flags & 0x08) > 0 then
      player.score = r:lstring()
    end

    if (flags & 0x10) > 0 then
      player.ping = r:lstring()
    end

    if (flags & 0x20) > 0 then
      player["time"] = r:lstring()
    end

    players:insert(player)
  end

  return players
end

local function get_pfields(players)
  local pfields = {
    {
      title = 'Name',
      kind = 'string',
      field = 'name',
    },
  }

  if #players > 0 then
    for k, v in pairs(players[0]) do
      pfields:insert({
        title = k:sub(1, 2):upper() .. k:sub(2),
        kind = 'string',
        field = k,
      })
    end
  end

  return pfields
end

local function normalize_server_info(details)
  local info = {}

  for k, v in pairs(info_map) do
    info[k] = details[v]
  end

  return info
end

function process(data)
  local r = DataReader(data)
  local sig = r:string(4)

  if sig ~= "EYE1" then
    error("invalid response: not All-Seeing Eye")
  end

  local general_info = read_general_info(r)
  local values = read_key_values(r)
  local details = concat_tables(general_info, values)
  local inf = normalize_server_info(general_info)
  local players = read_player_list(r)
  local pfields = get_pfields(players)

  gsw.sinfo(details, inf)
  gsw.plist(pfields, players)
end
