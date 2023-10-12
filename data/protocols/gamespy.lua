local CompoundResponse = require("lib/CompoundResponse")
local gamespy = require("lib/gamespy")
local quake2 = require("quake2")

protocol = {
  id        = "gamespy",
  name      = "GameSpy",
  feature   = "query",
  transport = "udp",
}

---@type CompoundResponse
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

function process(data)
  local info = quake2.parse_info(data)
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

    local inf = gamespy.extract_server_info(details)
    gsw.sinfo(details, inf)

    local players = get_players(details)
    local pfields = get_pfields(players)
    gsw.plist(pfields, players)
  end
end
