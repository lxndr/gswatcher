-- https://developer.valvesoftware.com/wiki/Source_RCON_Protocol

local switch = require("lib/switch")
local DataWriter = require("lib/DataWriter")
local DataReader = require("lib/DataReader")

protocol = {
  id        = "source-console",
  name      = "Source Engine Remote Console",
  feature   = "console",
  transport = "tcp",
}

---@enum AuthorizationState
local AuthorizationState = {
  NONE        = 0,
  AUTHORIZING = 1,
  AUTHORIZED  = 2,
}

---@enum PacketType
local PacketType = {
  AUTH           = 3,
  AUTH_RESPONSE  = 2,
  EXEC_COMMAND   = 2,
  RESPONSE_VALUE = 0,
}

---@class Packet
---@field id integer
---@field type PacketType
---@field body string
---@field empty ""

---@type string[]
local pending_commands = {}
---@type AuthorizationState
local authorization_state = AuthorizationState.NONE
---@type integer
local requestId = 0
---@type string
local buffer = ""

---@param id integer
---@param type PacketType
---@param body string
local function create_packet(id, type, body)
  local length = #body + 10
  local w = DataWriter ()
  w:i32le(length)
  w:i32le(id)
  w:i32le(type)
  w:zstring(body)
  w:zstring('')
  return w.buf
end

local function send_pending_command()
  if authorization_state == AuthorizationState.AUTHORIZED and #pending_commands > 0 then
    requestId = requestId + 1
    local cmd = table.remove(pending_commands, 1)
    gsw.send(create_packet(requestId, PacketType.EXEC_COMMAND, cmd))
  end
end

---@param cmd string
---@param params { password: string }
function send_command(cmd, params)
  table.insert(pending_commands, cmd)

  switch (authorization_state) {
    [AuthorizationState.NONE] = function()
      gsw.send(create_packet(requestId, PacketType.AUTH, params.password))
    end,
    [AuthorizationState.AUTHORIZED] = function()
      send_pending_command()
    end,
  }
end

---@param buf string
---@return Packet
local function parse_packet(buf)
  local r = DataReader(buf)

  return {
    id = r:i32le(),
    type = r:i32le(),
    body = r:zstring(),
    empty = r:zstring(),
  }
end

---@param pak Packet
local function process_auth_response(pak)
  if pak.id == requestId then
    authorization_state = AuthorizationState.AUTHORIZED
    send_pending_command()
  elseif pak.id == -1 then
    error("AuthError: authorization failed")
  else
    error("InvalidResponseError: invalid auth response")
  end
end

---@param pak Packet
local function process_response(pak)
  if pak.id == requestId then
    if authorization_state == AuthorizationState.AUTHORIZED then
      gsw.response(pak.body)
      send_pending_command()
    end
  else
    error("InvalidResponseError: invalid response id")
  end
end

---@param data string
function process(data)
  buffer = buffer .. data

  while #buffer >= 4 do
    local length = string.unpack("<I4", buffer)
    assert(length <= 4096, "invalid response packet length")

    if length > (#buffer - 4) then
      break
    end

    local pak = parse_packet(buffer:sub(5, length + 5))
    assert(pak.empty == "", "no empty trailing string")

    switch (pak.type) {
      [PacketType.AUTH_RESPONSE] = function()
        process_auth_response(pak)
      end,
      [PacketType.RESPONSE_VALUE] = function()
        process_response(pak)
      end,
      default = function()
        error("InvalidResponseError: invalid response type")
      end,
    }

    buffer = buffer:sub(length + 5)
  end
end
