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

local CompoundDictionaryResponse = require("lib/CompoundDictionaryResponse")
local gamespy = require("lib/gamespy")
local quake2 = require("quake2")

---@type ProtocolInfo
protocol = {
  id        = "gamespy",
  name      = "GameSpy",
  feature   = "query",
  transport = "udp",
}

---@type CompoundDictionaryResponse
local response = nil

---@param values Dictionary
---@return Buffer
local function create_packet(values)
  local data = ""

  for k, v in pairs(values) do
    data = data .. "\\" .. k .. "\\" .. v
  end

  return data
end

function query()
  response = CompoundDictionaryResponse()

  gsw.send(create_packet({
    basic = "",
    players = "",
    info = "",
  }))
end

---@param inf Dictionary
---@return Dictionary[]
local function get_players(inf)
  local players = {}

  for k, v in pairs(inf) do
    local _, _, key, idx = k:find("^(%a+)_(%d+)$")

    if key then
      local index = tonumber(idx) + 1
      local player = players[index]

      if player == nil then
        player = {}
        players[index] = player
      end

      player[key] = v
    end
  end

  return players
end

---@param players Dictionary[]
---@return PlayerField[]
local function get_pfields(players)
  local pfields = {}

  if #players > 0 then
    for k, v in pairs(players[1]) do
      table.insert(pfields, {
        title = k:sub(1, 1):upper() .. k:sub(2),
        kind = 'string',
        field = k,
      })
    end
  end

  return pfields
end

---@param data Buffer
function process(data)
  local info = quake2.parse_info(data)
  local queryid = info.queryid

  if not queryid then
    error("invalid response: no queryid found")
  end

  local reqid, fragment = queryid:match("^(%d+).(%d+)$")

  if not reqid then
    error("invalid response: invalid queryid")
  end

  ---@type Packet
  local pak = {
    reqid = tonumber(reqid),
    number = tonumber(fragment),
    data = info,
  }

  if info.final ~= nil then
    pak.total = pak.number
  end

  info.queryid = nil
  info.final = nil

  response:add_packet(pak)

  if response:got_all_packets() then
    local details = response:combine()

    local inf = gamespy.extract_server_info(details)
    gsw.sinfo(details, inf)

    local players = get_players(details)
    local pfields = get_pfields(players)
    gsw.plist(pfields, players)
  end
end
