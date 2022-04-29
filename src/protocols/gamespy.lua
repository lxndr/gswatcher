local to_boolean = require("lib/to_boolean")
local CompoundResponse = require("lib/CompoundResponse")
local quake = require("quake")

protocol = {
  id        = "gamespy",
  name      = "GameSpy",
  feature   = "query",
  transport = "udp",
}

local response = nil

local function create_packet(values)
  local data = ""

  for k, v in pairs(values) do
    data = data .. "\\" .. k .. "\\" .. v
  end

  return data
end

function query()
  response = CompoundResponse()

  gsw.send(create_packet({
    basic = "",
    players = "",
    info = "",
  }))
end

local function normalize_server_info(inf)
  return {
    server_name = inf.hostname,
    num_players = tonumber(inf.numplayers),
    max_players = tonumber(inf.maxplayers),
    map = inf.maptitle or inf.mapname,
    game_version = inf.gamever,
    game_mode = inf.gametype or inf.gamemode,
    private = to_boolean(inf.password),
  }
end

local function get_players(inf)
  local players = {}

  for k, v in pairs(inf) do
    local _, _, key, val = v:find("^(%a+)_(%d+)$")

    if key then
      local index = tonumber(val)
      local player = players[index]

      if not player then
        player = {}
        players[index] = player
      end

      player[key] = val
    end
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

function process(data)
  local info = quake.parse_info(data)
  local queryid = info.queryid

  if not queryid then
    error("no queryid found")
  end

  local reqid, fragment = queryid:match("^(%d+).(%d+)$")

  if not reqid then
    error("invalid queryid")
  end

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

    local inf = normalize_server_info(details)
    gsw.sinfo(details, inf)

    local players = get_players(details)
    local pfields = get_pfields(players)
    gsw.plist(pfields, players)
  end
end
