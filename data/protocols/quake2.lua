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

---@meta

local to_boolean = require("lib/to_boolean")
local DataReader = require("lib/DataReader")
local DataWriter = require("lib/DataWriter")

---@type ProtocolInfo
protocol = {
  id        = "quake2",
  name      = "Quake 2",
  feature   = "query",
  transport = "udp",
}

local pfields = {
  {
    title = "Name",
    kind = "string",
    field = "name",
  },
  {
    title = "Score",
    kind = "number",
    field = "score",
  },
  {
    title = "Ping",
    kind = "number",
    field = "ping",
  },
}

function query()
  local w = DataWriter()
  w:i32le(-1)
  w:string("status")
  gsw.send(w.buf)
end

local function normalize_server_info(details, plist)
  return {
    server_name = details.hostname or details.sv_hostname,
    game_name = details.gamename,
    game_mode = details.g_gametype,
    game_version = details.version or details.shortversion,
    map = details.mapname,
    num_players = #plist,
    max_players = tonumber(details.maxclients or details.sv_maxclients),
    private = to_boolean(details.g_needpass),
  }
end

---@param data string
local function parse_info(data)
  local values = {}
  local pos = 1

  while true do
    local s, e, key, val = data:find("^\\([%w_.]+)\\([^\\]*)", pos)

    if s == nil then
        break
    end

    values[key] = val
    pos = e + 1
  end

  return values
end

---@param str string
local function parse_player(str)
  local s, e, score, ping, name = str:find("^(%d+) (%d+) \"(.+)\"$")

  if s == nil then
    error("invalid response: invalid player record")
  end

  return {
    name = name,
    score = score,
    ping = ping,
  }
end

---@param data Buffer
---@param header integer
---@param signature string
local function process_response(data, header, signature)
  local r = DataReader(data)
  local hdr = r:i32le()

  if hdr ~= header then
    error("invalid response: invalid header")
  end

  local parts = {}

  while not r:is_end() do
    local part = r:zstring("\n")
    table.insert(parts, part)
  end

  if parts[1] ~= signature then
    error("invalid response: invalid packet")
  end

  local details = parse_info(parts[2])
  local players = {}

  for idx = 3, #parts do
    table.insert(players, parse_player(parts[idx]))
  end

  local inf = normalize_server_info(details, players)

  gsw.sinfo(details, inf)
  gsw.plist(pfields, players)
end

---@param data Buffer
function process(data)
  process_response(data, -1, "print")
end

return {
  parse_info = parse_info,
  process_response = process_response,
}
