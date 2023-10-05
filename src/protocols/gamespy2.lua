local to_boolean = require("lib/to_boolean")
local DataWriter = require("lib/DataWriter")
local DataReader = require("lib/DataReader")

protocol = {
  id        = "gamespy2",
  name      = "GameSpy 2",
  feature   = "query",
  transport = "udp",
}

---@class Field
---@field title string
---@field kind string
---@field field string

---@type integer
local requestId = 0

function query()
  local w = DataWriter ()
    :u8(0xfe):u8(0xfd) -- header
    :u8(0x00)          -- delimiter
    :u32le(requestId)  -- request id
    :u8(0xff)          -- get server info and rules
    :u8(0xff)          -- get player info
    :u8(0xff)          -- get team info

  gsw.send(w.buf)
end

--- @param details table
local function normalize_server_info(details)
  return {
    server_name = details.hostname,
    num_players = tonumber(details.numplayers),
    max_players = tonumber(details.maxplayers),
    map = details.maptitle or details.mapname,
    game_version = details.gamever,
    game_mode = details.gametype or details.gamemode,
    secure = to_boolean(details.sv_punkbuster),
  }
end

--- @param r DataReader
--- @param delimiter integer
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

--- @param r DataReader
--- @param delimiter integer
local function read_player_fields(r, delimiter)
  ---@type Field[]
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

--- @param r DataReader
--- @param delimiter integer
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

---@param data string
function process(data)
  local r = DataReader(data)
  local delimiter = r:u8()
  local reqid = r:u32le()

  if reqid ~= requestId then
    error("InvalidResponseError: invalid request id")
  end

  local details = parse_server_info(r, delimiter)
  local pfields, players = parse_player_list(r, delimiter)
  local inf = normalize_server_info(details)

  gsw.sinfo(details, inf)
  gsw.plist(pfields, players)
end
