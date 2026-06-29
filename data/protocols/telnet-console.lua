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

---@type ProtocolInfo
protocol = {
  id        = "telnet",
  name      = "Telnet Console",
  feature   = "console",
  transport = "tcp",
}

---@enum AuthenticationState
local AuthenticationState = {
  NONE          = 0,
  AUTHENTICATING = 1,
  AUTHORIZED    = 2,
}

---@type AuthenticationState
local authentication_state = AuthenticationState.NONE
---@type string
local buffer = ""
---@type string?
local pending_command = nil
---@type string
local auth_password = ""
---@type boolean
local connection_initiated = false

local IAC = string.char(255)

---@param data string
---@return string
local function strip_telnet_sequences(data)
  local parts = {}
  local i = 1

  while i <= #data do
    if data:byte(i) == 255 then
      if i + 1 <= #data and data:byte(i + 1) == 255 then
        parts[#parts + 1] = IAC
        i = i + 2
      elseif i + 2 <= #data then
        i = i + 3
      else
        i = i + 1
      end
    else
      parts[#parts + 1] = data:sub(i, i)
      i = i + 1
    end
  end

  return table.concat(parts)
end

---@param line string
---@return boolean
local function is_auth_failure(line)
  local lower = line:lower()
  return lower:find("failed", 1, true) ~= nil
    or lower:find("denied", 1, true) ~= nil
    or lower:find("invalid", 1, true) ~= nil
    or lower:find("incorrect", 1, true) ~= nil
end

---@param line string
local function is_password_prompt(line)
  return line:lower():find("password", 1, true) ~= nil
end

---@param line string
---@return boolean
local function should_skip_line(line)
  return line == ""
end

local function ensure_connected()
  if not connection_initiated then
    connection_initiated = true
    -- Empty line opens TCP; servers ignore blank lines during authentication.
    gsw.send("\n")
  end
end

local function authorize()
  authentication_state = AuthenticationState.AUTHORIZED

  if pending_command then
    local cmd = pending_command
    pending_command = nil
    gsw.send(cmd .. "\n")
  end
end

---@param line string
local function process_line(line)
  if should_skip_line(line) then
    return
  end

  if authentication_state == AuthenticationState.AUTHENTICATING then
    gsw.response(line)

    if is_auth_failure(line) then
      error("AuthError: authentication failed")
    end

    authorize()
    return
  end

  if authentication_state == AuthenticationState.NONE then
    if is_password_prompt(line) then
      gsw.response(line)
      authentication_state = AuthenticationState.AUTHENTICATING
      gsw.send(auth_password .. "\n")
      return
    end

    gsw.response(line)
    authorize()
    return
  end

  gsw.response(line)
end

---@param data string
local function process_lines(data)
  buffer = buffer .. data

  while true do
    local pos = buffer:find("\n", 1, true)

    if pos == nil then
      break
    end

    local line = buffer:sub(1, pos - 1)
    buffer = buffer:sub(pos + 1)

    if #line > 0 and line:sub(-1) == "\r" then
      line = line:sub(1, -2)
    end

    process_line(line)
  end
end

---@param cmd string
---@param params { password: string }
function send_command(cmd, params)
  auth_password = params.password or ""

  if authentication_state == AuthenticationState.AUTHORIZED then
    gsw.send(cmd .. "\n")
  else
    pending_command = cmd
    ensure_connected()
  end
end

---@param data Buffer
function process(data)
  process_lines(strip_telnet_sequences(data))
end
