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

local rand_integer = require("lib/rand_integer")
local DataWriter = require("lib/DataWriter")
local DataReader = require("lib/DataReader")
local gamespy = require("lib/gamespy")

---@type ProtocolInfo
protocol = {
  id        = "gamespy2",
  name      = "GameSpy 2",
  feature   = "query",
  transport = "udp",
}

---@type integer
local requestId = 0

function query()
  requestId = rand_integer(1, 0xffffffff)

  local w = DataWriter ()
    :u8(0xfe):u8(0xfd) -- header
    :u8(0x00)          -- delimiter
    :u32le(requestId)  -- request id
    :u8(0xff)          -- get server info and rules
    :u8(0xff)          -- get player info
    :u8(0xff)          -- get team info

  gsw.send(w.buf)
end

---@param r DataReader
---@param delimiter string
---@return Dictionary
local function parse_server_info(r, delimiter)
  local details = {}

  while not r:is_end() do
    local key = r:zstring(delimiter)
    local val = r:zstring(delimiter)

    if #key == 0 and #val == 0 then
      break
    end

    details[key] = val
  end

  return details
end

---@param r DataReader
---@param delimiter string
---@return PlayerField[]
local function read_player_fields(r, delimiter)
  local fields = {}

  while not r:is_end() do
    local field = r:zstring(delimiter)

    if #field == 0 then
      break
    end

    table.insert(fields, {
      title = field:sub(1, 1):upper() .. field:sub(2, -2),
      kind = "string",
      field = field,
    })
  end

  return fields
end

---@param r DataReader
---@param delimiter string
---@return PlayerField[], Dictionary[]
local function parse_player_list(r, delimiter)
  local players = {}
  local player_count = r:u8()
  local pfields = read_player_fields(r, delimiter)

  for player_idx = 1, player_count do
    if r:is_end() then
      break
    end

    local player = {}

    for field_idx = 1, #pfields do
      local val = r:zstring(delimiter)
      player[pfields[field_idx].field] = val
    end

    players[player_idx] = player
  end

  return pfields, players
end

---@param data Buffer
function process(data)
  local r = DataReader(data)
  local delimiter = string.char(r:u8())
  local reqid = r:u32le()

  if reqid ~= requestId then
    error("invalid response: invalid request id")
  end

  local details = parse_server_info(r, delimiter)
  local pfields, players = parse_player_list(r, delimiter)
  local inf = gamespy.extract_server_info(details)

  gsw.sinfo(details, inf)
  gsw.plist(pfields, players)
end
