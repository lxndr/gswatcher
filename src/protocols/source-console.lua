local switch = require("lib/switch")
local DataWriter = require("lib/DataWriter")
local DataReader = require("lib/DataReader")

protocol = {
  id        = "source-console",
  name      = "Source Engine Remote Console",
  feature   = "console",
  transport = "tcp",
}

local PacketType = {
  AUTH           = 3,
  AUTH_RESPONSE  = 2,
  EXEC_COMMAND   = 2,
  RESPONSE_VALUE = 0,
}

local pending_command = nil
local authorized = false
local requestId = 0
local buffer = ""

---@param id string
---@param type integer
---@param body string
function create_packet(id, type, body)
  local length = #body + 10
  local w = DataWriter ()
  w:i32le(length)
  w:i32le(id)
  w:i32le(type)
  w:zstring(body)
  w:zstring('')
  return w.buf
end

---@param cmd string
---@param options table
function send_command(cmd, options)
  if options.authorized then
    gsw.send(create_packet(requestId, PacketType.EXEC_COMMAND, cmd))
  else
    pending_command = cmd
    gsw.send(create_packet(requestId, PacketType.AUTH, options.password))
  end
end

---@param buf string
function parse_packet(buf)
  local r = DataReader(buf)

  return {
    id = r:i32le(),
    type = r:i32le(),
    body = r:zstring(),
    empty = r:zstring(),
  }
end

function process_auth_response(pak)
  if pak.id == requestId then
    authorized = true

    if pending_command then
      send_command(pending_command)
    end
  elseif pak.id == -1 then
    error("unauthorized")
  else
    error("invalid auth response")
  end
end

function process_response(pak)
  if pak.id == requestId then
    gsw.response(pak.body)
  else
    error("invalid response id")
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

    local pak = parse_packet(buffer:sub(5, length + 4))
    assert(pak.empty == "", "no empty trailing string")

    switch (pak.type) {
      [PacketType.AUTH_RESPONSE] = function ()
        process_auth_response(pak)
      end,
      [PacketType.RESPONSE_VALUE] = function ()
        process_response(pak)
      end,
      default = function ()
        error("invalid response type")
      end,
    }

    buffer = buffer:sub(length + 4)
  end
end
