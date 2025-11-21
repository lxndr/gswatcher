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

-- https://developer.valvesoftware.com/wiki/Server_Queries

---@class PacketHeader: Packet
---@field compressed boolean
---@field data_size? number
---@field crc32? number

---@class SourceServerInfo
---@field protocol_version integer
---@field address? string
---@field name string
---@field map string
---@field folder string
---@field game string
---@field appid integer
---@field num_players integer
---@field max_players integer
---@field num_bots integer
---@field server_type string
---@field environment string
---@field visibility integer
---@field vac integer
---@field version string
---@field port? integer
---@field steamid? integer
---@field sourcetv_port? integer
---@field sourcetv_name? integer
---@field keywords? string
---@field gameid? integer
---@field the_ship_mode? integer
---@field the_ship_witnesses? integer
---@field the_ship_duration? integer

local switch = require("lib/switch")
local to_boolean = require("lib/to_boolean")
local DataReader = require("lib/DataReader")
local DataWriter = require("lib/DataWriter")
local CompoundBufferResponse = require("lib/CompoundBufferResponse")

---@type ProtocolInfo
protocol = {
  id        = "source",
  name      = "Source Engine",
  feature   = "query",
  transport = "udp",
}

---@enum RequestPacketType
local RequestPacketType = {
  SERVER_INFO = "T", -- 0x54
  PLAYER_LIST = "U", -- 0x55
}

---@type Dictionary
local info_map = {
  game_name    = "game",
  game_version = "version",
  server_name  = "name",
  server_type  = "server_type",
  server_os    = "environment",
  map          = "map",
  num_players  = "num_players",
  max_players  = "max_players",
  private      = "visibility",
  secure       = "vac",
}

---@type table<string, integer>
local known_app_ids = {
  source_sdk_base_2006 = 215,
  counter_strike_source = 240,
  the_ship = 2400,
  eternal_silence = 17550,
  insurgency = 17700,
}

---@type CompoundBufferResponse
local response = CompoundBufferResponse()

---@type boolean
local is_gold_source = false

---@type integer
local got_challenge = -1

---@type SourceServerInfo|nil
local got_server_info = nil

---@type Player[]|nil
local got_player_list = nil

---@param type RequestPacketType
---@param payload? Buffer
local function send_packet(type, payload)
  local w = DataWriter()

  w:i32le(-1)
  w:string(type, 1)

  if payload then
    w:data(payload)
  end

  gsw.send(w.buf)
end

---@param challenge integer
local function send_server_info_packet(challenge)
  local w = DataWriter()
  w:zstring("Source Engine Query")
  w:i32le(challenge)

  send_packet(RequestPacketType.SERVER_INFO, w.buf)
end

---@param challenge integer
local function send_player_list_packet(challenge)
  local w = DataWriter()

  if challenge then
    w:i32le(challenge)
  end

  send_packet(RequestPacketType.PLAYER_LIST, w.buf)
end

local function next_query()
  response = CompoundBufferResponse()

  if not got_server_info then
    send_server_info_packet(got_challenge)
    return
  end

  if not got_player_list then
    send_player_list_packet(got_challenge)
  end
end

function query()
  is_gold_source = false
  got_challenge = -1
  got_server_info = nil
  got_player_list = nil
  next_query()
end

---@param r DataReader
---@return SourceServerInfo
local function read_server_info_gold(r)
  local inf = {
    address = r:zstring(),
    name = r:zstring(),
    map = r:zstring(),
    folder = r:zstring(),
    game = r:zstring(),
    num_players = r:u8(),
    max_players = r:u8(),
    num_bots = 0,
    protocol_version = r:u8(),
    server_type = r:string(1),
    environment = r:string(1),
    visibility = r:u8(),
  }

  local mod = r:u8()

  if mod then
    inf.mod = {
      link = r:zstring(),
      download = r:zstring(),
      null = r:u8(),
      version = r:i32le(),
      size = r:i32le(),
    }
  end

  inf.vac = r:u8()
  inf.num_bots = r:u8()

  return inf
end

---@param r DataReader
---@return SourceServerInfo
local function read_server_info(r)
  local inf = {
    protocol_version = r:u8(),
    name = r:zstring(),
    map = r:zstring(),
    folder = r:zstring(),
    game = r:zstring(),
    appid = r:u16le(),
    num_players = r:u8(),
    max_players = r:u8(),
    num_bots = r:u8(),
    server_type = r:string(1),
    environment = r:string(1),
    visibility = r:u8(),
    vac = r:u8(),
  }

  if inf.appid == known_app_ids.the_ship then
    inf.the_ship_mode = r:u8()
    inf.the_ship_witnesses = r:u8()
    inf.the_ship_duration = r:u8()
  end

  inf.version = r:zstring()

  if r:is_end() then
    return inf
  end

  local edf = r:u8()

  if (edf & 0x80) > 0 then
    inf.port = r:u16le()
  end

  if (edf & 0x10) > 0 then
    inf.steamid = r:u64le()
  end

  if (edf & 0x40) > 0 then
    inf.sourcetv_port = r:u16le()
    inf.sourcetv_name = r:zstring()
  end

  if (edf & 0x20) > 0 then
    inf.keywords = r:zstring()
  end

  if (edf & 0x01) > 0 then
    inf.gameid = r:u64le()
  end

  return inf
end

---@param details Dictionary
---@return ServerInfo
local function normalize_server_info(details)
  local info = {}

  for k, v in pairs(info_map) do
    info[k] = details[v]
  end

  return info
end

---@param inf Dictionary
---@return PlayerField[]
local function create_player_fields(inf)
  ---@type PlayerField[]
  local fields = {
    {
      title = "Name",
      kind = "string",
      field = "name",
      main = true,
    },
    {
      title = "Score",
      kind = "number",
      field = "score",
    },
    {
      title = "Time",
      kind = "duration",
      field = "duration",
    },
  }

  if inf.appid == known_app_ids.the_ship then
    table.insert(fields, {
      title = "Deaths",
      kind = "number",
      field = "deaths",
    })

    table.insert(fields, {
      title = "Money",
      kind = "number",
      field = "money",
    })
  end

  return fields
end

---@param r DataReader
---@param inf SourceServerInfo
local function read_player_list(r, inf)
  local players = {}
  local count = r:u8()

  for idx = 1, count do
    players[idx] = {
      index = r:u8(),
      name = r:zstring(),
      score = r:i32le(),
      duration = r:f32le(),
    }
  end

  if inf.appid == known_app_ids.the_ship then
    for idx = 1, count do
      players[idx].deaths = r:u32le()
      players[idx].money = r:u32le()
    end
  end

  return players
end

---@param r DataReader
local function read_challenge(r)
  return r:i32le()
end

---@param r DataReader
local function read_payload(r)
  local type = r:string(1)

  switch (type) {
    I = function ()
      got_challenge = -1
      got_server_info = read_server_info(r)
      local normalized_server_info = normalize_server_info(got_server_info)
      gsw.sinfo(got_server_info, normalized_server_info)
    end,
    m = function ()
      got_challenge = -1
      got_server_info = read_server_info_gold(r)
      is_gold_source = true
      local normalized_server_info = normalize_server_info(got_server_info)
      gsw.sinfo(got_server_info, normalized_server_info)
    end,
    D = function ()
      got_challenge = -1

      if not got_server_info then
        error("invalid response: got player list before server info")
      end

      got_player_list = read_player_list(r, got_server_info)
      local pfields = create_player_fields(got_server_info)
      gsw.plist(pfields, got_player_list)
    end,
    A = function ()
      got_challenge = read_challenge(r)
    end,
    default = function ()
      error("invalid response: unexpected response type " .. type)
    end,
  }

  next_query ()
end

---@param r DataReader
---@return PacketHeader
local function read_header_gold(r)
  local reqid = r:u32le()
  local byte = r:u8()
  local total = byte & 0x0f
  local number = (byte >> 4) + 1

  if total < 2 then
    error("total number of packets (" .. total .. ") less than 2")
  end

  if total > 15 then
    error("total number of packets (" .. total .. ") more than 15")
  end

  if number > total then
    error("packet number (" .. number .. ") is greater then total number of packets (" .. total .. ")")
  end

  local data = r:data()

  return {
    reqid = reqid,
    total = total,
    number = number,
    data = data,
  }
end

---@param r DataReader
---@return PacketHeader
local function read_header(r)
  local packet = {
    reqid = r:u32le(),
    total = r:u8(),
    number = r:u8() + 1,
    size = r:u16le(), -- FIXME: Orange Box Engine and above only.
  }

  if packet.total < 2 then
    error("total number of packets (" .. packet.total .. ") less than 2")
  end

  if packet.number > packet.total then
    error("packet number (" .. packet.number .. ") is greater then total number of packets (" .. packet.total .. ")")
  end

  packet.compressed = to_boolean(packet.reqid >> 31)

  if packet.number == 1 and packet.compressed then
    packet.data_size = r:u32le()
    packet.crc32 = r:u32le()
  end

  packet.data = r:data()
  return packet
end

---@param r DataReader
---@return PacketHeader
local function read_packet(r)
  local format = r:i32le()

  if format == -1 then
    return {
      reqid = 0,
      total = 1,
      number = 1,
      compressed = false,
      data = r:data(),
    }
  end

  if format == -2 then
    if is_gold_source then
      return read_header_gold(r)
    else
      return read_header(r)
    end
  end

  error("invalid response: unknown packet format " .. format)
end

---@param data Buffer
function process(data)
  local reader = DataReader(data)
  local packet = read_packet(reader)

  if not response.reqid then
    response.reqid = packet.reqid
  end

  response:add_packet(packet)

  if not response:got_all_packets() then
    return
  end

  data = response:combine()

  local first_packet = response.packets[1]
  assert(not first_packet.compressed, "compressed split packets are not supported")

  -- local uncompressed_data = bz2:uncompress(data)

  -- if #uncompressed_data ~= first_packet.size then
  --   error("invalid response: invalid uncompressed data size")
  -- end

  -- if crc32:hash(uncompressed_data) ~= first_packet.crc32 then
  --   error("invalid response: failed to crc32 validate uncompressed data")
  -- end

  reader = DataReader(data)

  -- Some titles when using the split packet method have been observed erroneously nesting the single message header inside the split payload
  if response.total > 1 and #reader.buf >= 4 and reader:i32le() ~= -1 then
    reader:skip(-4)
  end

  read_payload(reader)
end
