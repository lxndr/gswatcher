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

local switch = require("lib/switch")
local to_boolean = require("lib/to_boolean")
local DataWriter = require("lib/DataWriter")

---@param d table Server variables (details)
---@return table<string, string|integer|boolean>
local function extract_server_info(d)
  return {
    server_name = d.hostname,
    num_players = tonumber(d.numplayers),
    max_players = tonumber(d.maxplayers),
    map = d.mapname or d.maptitle,
    game_name = d.gamename or d.game_id,
    game_version = d.gamever or d.version,
    game_mode = d.gametype or d.gamemode,
    private = to_boolean(d.password),
    secure = to_boolean(d.sv_punkbuster),
  }
end

---@param r DataReader
---@return string[]
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

---@param request_id integer
---@return Buffer
local function create_challenge_request(request_id)
  return DataWriter()
    :u8(0xfe):u8(0xfd)
    :u8(9)
    :i32be(request_id)
    .buf
end

---@param request_id integer
---@param challenge? integer
---@return Buffer
local function create_stats_request(request_id, challenge)
  local w = DataWriter()
    :u8(0xfe):u8(0xfd)
    :u8(0)
    :i32be(request_id)

  if challenge ~= nil then
    w:i32be(challenge)
  end

  w:u32be(0xffffff01)
  return w.buf
end

---@param pfields PlayerField[]
---@param field_id string
local function add_player_field(pfields, field_id)
  for _, field in ipairs(pfields) do
    if field.field == field_id then
      return
    end
  end

  table.insert(pfields, {
    title = field_id:sub(1, 1):upper() .. field_id:sub(2, -2),
    kind = "string",
    field = field_id,
  })
end

---@param r DataReader
---@param response CompoundDictionaryResponse
---@param response_id integer
local function parse_v3_stats_response(r, response, response_id)
  local splitnum = r:zstring()

  if splitnum ~= "splitnum" then
    error("invalid response: invalid 'splitnum'")
  end

  local num_packets = r:u8()
  local final = (num_packets & 0x80) > 0
  local number = (num_packets & 0x7f) + 1

  local packet = {
    reqid = response_id,
    number = number,
    data = {}
  }

  if final then
    packet.total = number
  end

  while not r:is_end() do
    local info_type = r:u8()

    switch (info_type) {
      -- server info
      [0] = function()
        while not r:is_end() do
          local key = r:zstring()

          if #key == 0 then
            break
          end

          local val = r:zstring()
          response.details[key] = val
        end
      end,
      -- player info
      [1] = function()
        while not r:is_end() do
          local field = r:zstring()

          if #field == 0 then
            break
          end

          add_player_field(response.pfields, field)
          local idx = r:u8() + 1

          while not r:is_end() do
            local val = r:zstring()

            if #val == 0 then
              break
            end

            if response.plist[idx] == nil then
              response.plist[idx] = {}
            end

            response.plist[idx][field] = val
            idx = idx + 1
          end
        end
      end,
      -- team info
      [2] = function()
        while not r:is_end() do
          local field = r:zstring()

          if #field == 0 then
            break
          end

          local idx = r:u8() + 1

          while not r:is_end() do
            local val = r:zstring()

            if #val == 0 then
              break
            end
          end
        end
      end,
      default = function()
        error("invalid response: invalid info type " .. info_type)
      end,
    }
  end

  response:add_packet(packet)

  if response:got_all_packets() then
    local sinfo = extract_server_info(response.details)
    return response.details, sinfo, response.pfields, response.plist
  end

  return nil
end

return {
  extract_server_info = extract_server_info,
  read_string_list = read_string_list,
  create_challenge_request = create_challenge_request,
  create_stats_request = create_stats_request,
  parse_v3_stats_response = parse_v3_stats_response,
}
