---@meta

local to_boolean = require("lib/to_boolean")
local DataReader = require("lib/DataReader")

---@param d table Server variables (details)
local function extract_server_info(d)
  return {
    server_name = d.hostname,
    num_players = tonumber(d.numplayers),
    max_players = tonumber(d.maxplayers),
    map = d.maptitle or d.mapname,
    game_version = d.gamever,
    game_mode = d.gametype or d.gamemode,
    private = to_boolean(d.password),
    secure = to_boolean(d.sv_punkbuster),
  }
end

---@param r DataReader
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

return {
  extract_server_info = extract_server_info,
  read_string_list = read_string_list,
}
