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

-- https://developer.valvesoftware.com/wiki/Source_RCON_Protocol

local switch = require("lib/switch")
local DataWriter = require("lib/DataWriter")
local DataReader = require("lib/DataReader")

---@type ProtocolInfo
protocol = {
  id        = "source-console",
  name      = "Source Engine Remote Console",
  feature   = "console",
  transport = "tcp",
}

---@enum AuthenticationState
local AuthenticationState = {
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

---@class SourcePacket
---@field id integer
---@field type PacketType
---@field body string
---@field empty ""

---@type string[]
local pending_commands = {}
---@type AuthenticationState
local authentication_state = AuthenticationState.NONE
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
  if authentication_state == AuthenticationState.AUTHORIZED and #pending_commands > 0 then
    requestId = requestId + 1
    local cmd = table.remove(pending_commands, 1)
    gsw.send(create_packet(requestId, PacketType.EXEC_COMMAND, cmd))
  end
end

---@param cmd string
---@param params { password: string }
function send_command(cmd, params)
  table.insert(pending_commands, cmd)

  switch (authentication_state) {
    [AuthenticationState.NONE] = function()
      gsw.send(create_packet(requestId, PacketType.AUTH, params.password))
    end,
    [AuthenticationState.AUTHORIZED] = function()
      send_pending_command()
    end,
  }
end

---@param buf string
---@return SourcePacket
local function parse_packet(buf)
  local r = DataReader(buf)

  return {
    id = r:i32le(),
    type = r:i32le(),
    body = r:zstring(),
    empty = r:zstring(),
  }
end

---@param pak SourcePacket
local function process_auth_response(pak)
  if pak.id == requestId then
    authentication_state = AuthenticationState.AUTHORIZED
    send_pending_command()
  elseif pak.id == -1 then
    error("AuthError: authentication failed")
  else
    error("invalid response: invalid authentication response")
  end
end

---@param pak SourcePacket
local function process_response(pak)
  if pak.id == requestId then
    if authentication_state == AuthenticationState.AUTHORIZED then
      gsw.response(pak.body)
      send_pending_command()
    end
  else
    error("invalid response: invalid response id")
  end
end

---@param data Buffer
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
        error("invalid response: invalid response type")
      end,
    }

    buffer = buffer:sub(length + 5)
  end
end
