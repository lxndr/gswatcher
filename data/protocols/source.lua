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

-- https://developer.valvesoftware.com/wiki/Server_Queries

local switch = require("lib/switch")
local to_boolean = require("lib/to_boolean")
local DataReader = require("lib/DataReader")
local DataWriter = require("lib/DataWriter")
local CompoundResponse = require("lib/CompoundResponse")

protocol = {
  id        = "source",
  name      = "Source Engine",
  feature   = "query",
  transport = "udp",
}

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

local known_app_ids = {
  the_ship = 2400,
}

local response = nil
local got_challenge = -1
local got_server_info = nil
local got_player_list = nil

local function send_packet(type, payload)
  local w = DataWriter()

  w:i32le(-1)
  w:string(type, 1)

  if payload then
    w:data(payload)
  end

  gsw.send(w.buf)
end

local function send_server_info_packet(challenge)
  local w = DataWriter()
  w:zstring("Source Engine Query")
  w:i32le(challenge)

  send_packet("T", w.buf)
end

local function send_player_list_packet(challenge)
  local w = DataWriter()

  if challenge then
    w:i32le(challenge)
  end

  send_packet("U", w.buf)
end

local function next_query()
  if not got_server_info then
    send_server_info_packet(got_challenge)
    return
  end

  if not got_player_list then
    send_player_list_packet(got_challenge)
  end
end

function query()
  response = CompoundResponse()
  got_challenge = -1
  got_server_info = nil
  got_player_list = nil
  next_query()
end

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
    inf.steamid = tostring(r:u64le())
  end

  if (edf & 0x40) > 0 then
    inf.sourcetv = {
      port = r:u16le(),
      name = r:zstring(),
    }
  end

  if (edf & 0x20) > 0 then
    inf.keywords = r:zstring()
  end

  if (edf & 0x01) > 0 then
    inf.gameid = r:u64le()
  end

  return inf
end

local function normalize_server_info(details)
  local info = {}

  for k, v in pairs(info_map) do
    info[k] = details[v]
  end

  return info
end

local function create_player_fields(inf)
  local fields = {
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

local function read_challenge(r)
  return r:i32le()
end

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
      local normalized_server_info = normalize_server_info(got_server_info)
      gsw.sinfo(got_server_info, normalized_server_info)
    end,
    D = function ()
      got_challenge = -1
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

local function read_header_gold(r)
  local packet = {
    reqid = r:u32le(),
  }

  local number = r:u8()
  packet.total = number & 0x0f
  packet.number = (number >> 4) + 1

  packet.data = r:data()
  response:add_packet(packet)
end

local function read_header(r)
  local packet = {
    reqid = r:u32le(),
    total = r:u8(),
    number = r:u8() + 1,
    size = r:u16le(),
  }

  packet.compressed = to_boolean(packet.reqid >> 31)
  assert(not packet.compressed, "compressed split packets are not supported")

  if packet.number == 1 and packet.compressed then
    packet.dataSize = r:u32le()
    packet.crc32 = r:u32le()
  end

  packet.data = r:data()
  response:add_packet(packet)
end

local function try_read_header(r)
  local pos = r.pos
  local ok, pak = pcall(read_header_gold, r)

  if not ok then
    r.pos = pos
    pak = read_header(r)
  end

  return pak
end

function process(data)
  local r = DataReader(data)
  local format = r:i32le()

  switch (format) {
    [-1] = function ()
      read_payload(r)
    end,
    [-2] = function ()
      try_read_header(r)

      if response:got_all_packets() then
        local data = response:combine()
        local r = DataReader(data)
        read_payload(r)
      end
    end,
    default = function ()
      error("invalid response: unknown packet format " .. format)
    end,
  }
end
