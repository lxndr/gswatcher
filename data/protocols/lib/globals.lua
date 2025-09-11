---@meta

---@alias Buffer string Byte array.
---@alias Dictionary table<string, string>

---@alias ProtocolFeature 'query'|'console'
---@alias ProtocolTransport 'udp'|'tcp'

---@class (exact) ProtocolInfo
---@field id string Protocol ID.
---@field name string Protocol name.
---@field feature ProtocolFeature The feature that protocol implements.
---@field transport ProtocolTransport The transport that protocol uses.

---@class (exact) ServerInfo
---@field game_name string
---@field game_version string
---@field server_name string
---@field server_type string
---@field server_os string
---@field map string
---@field num_players string
---@field max_players string
---@field private string
---@field secure string

---@class (exact) PlayerField
---@field title string
---@field kind "string"|"number"|"duration"
---@field field string

---@alias Player Dictionary

gsw = {
  ---@param data string
  send = function (data) end,

  ---@param details Dictionary
  ---@param sinfo Dictionary
  sinfo = function (details, sinfo) end,

  ---@param pfields PlayerField[]
  ---@param plist Dictionary[]
  plist = function (pfields, plist) end,
}
