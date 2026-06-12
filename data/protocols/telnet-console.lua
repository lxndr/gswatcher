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

-- 7 Days to Die and similar servers expose a line-based TCP console (often
-- called "Telnet" in serverconfig.xml). See github.com/gorcon/telnet.

local switch = require("lib/switch")
local LineBuffer = require("lib/LineBuffer")

---@type ProtocolInfo
protocol = {
  id        = "telnet",
  name      = "Telnet Remote Console",
  feature   = "console",
  transport = "tcp",
}

local CRLF = "\r\n"

-- Strip Telnet (RFC 854) command sequences from the byte stream. Options are
-- logged and ignored; no negotiation replies are sent.

local IAC = string.char(255)

local CMD = {
  SE   = 240,
  NOP  = 241,
  DM   = 242,
  BRK  = 243,
  IP   = 244,
  AO   = 245,
  AYT  = 246,
  EC   = 247,
  EL   = 248,
  GA   = 249,
  SB   = 250,
  WILL = 251,
  WONT = 252,
  DO   = 253,
  DONT = 254,
  IAC  = 255,
}

local CMD_NAME = {
  [CMD.SE]   = "SE",
  [CMD.NOP]  = "NOP",
  [CMD.DM]   = "DM",
  [CMD.BRK]  = "BRK",
  [CMD.IP]   = "IP",
  [CMD.AO]   = "AO",
  [CMD.AYT]  = "AYT",
  [CMD.EC]   = "EC",
  [CMD.EL]   = "EL",
  [CMD.GA]   = "GA",
  [CMD.SB]   = "SB",
  [CMD.WILL] = "WILL",
  [CMD.WONT] = "WONT",
  [CMD.DO]   = "DO",
  [CMD.DONT] = "DONT",
  [CMD.IAC]  = "IAC",
}

---@type string
local telnet_pending = ""

---@param cmd integer
---@return string
local function telnet_cmd_name(cmd)
  return CMD_NAME[cmd] or ("CMD_" .. tostring(cmd))
end

---@param cmd integer
---@param option? integer
local function telnet_log_command(cmd, option)
  if option then
    gsw.log("telnet: " .. telnet_cmd_name(cmd) .. " " .. tostring(option) .. " (ignored)")
  else
    gsw.log("telnet: " .. telnet_cmd_name(cmd) .. " (ignored)")
  end
end

---@param data string
---@return string
local function telnet_process(data)
  telnet_pending = telnet_pending .. data
  local text = {}

  local i = 1

  while i <= #telnet_pending do
    local byte = telnet_pending:byte(i)

    if byte == CMD.IAC then
      if i == #telnet_pending then
        break
      end

      local cmd = telnet_pending:byte(i + 1)

      if cmd == CMD.IAC then
        table.insert(text, string.char(255))
        i = i + 2
      elseif cmd == CMD.WILL or cmd == CMD.WONT or cmd == CMD.DO or cmd == CMD.DONT then
        if i + 2 > #telnet_pending then
          break
        end

        local option = telnet_pending:byte(i + 2)
        telnet_log_command(cmd, option)
        i = i + 3
      elseif cmd == CMD.SB then
        local se = telnet_pending:find(IAC .. string.char(CMD.SE), i + 2, true)

        if not se then
          break
        end

        local payload = telnet_pending:sub(i + 2, se - 1)
        gsw.log("telnet: SB " .. #payload .. " bytes (ignored)")
        i = se + 2
      else
        telnet_log_command(cmd)
        i = i + 2
      end
    elseif byte == 0 then
      i = i + 1
    else
      table.insert(text, string.char(byte))
      i = i + 1
    end
  end

  telnet_pending = telnet_pending:sub(i)
  return table.concat(text)
end

---@enum AuthenticationState
local AuthenticationState = {
  NONE        = 0,
  AWAITING    = 1,
  AUTHORIZED  = 2,
}

local PROMPT_PASSWORD             = "Please enter password"
local RESPONSE_AUTH_SUCCESS       = "Logon successful."
local RESPONSE_AUTH_FAIL          = "Password incorrect"
local RESPONSE_AUTH_TOO_MANY      = "Too many failed login attempts!"
local RESPONSE_WELCOME            = "Press 'help' to get a list of all commands"
local RESPONSE_INF_PREFIX         = "INF Executing command"

---@type AuthenticationState
local authentication_state = AuthenticationState.NONE
---@type string
local stored_password = ""
---@type string[]
local pending_commands = {}
---@type LineBuffer
local line_buffer = LineBuffer()
---@type boolean
local awaiting_command_response = false

---@param line string
---@return boolean
local function is_login_or_auth_line(line)
  if line == "" then
    return true
  end

  if line:find(PROMPT_PASSWORD, 1, true) then
    return true
  end

  if line:find(RESPONSE_AUTH_SUCCESS, 1, true) then
    return true
  end

  if line:find(RESPONSE_AUTH_FAIL, 1, true) then
    return true
  end

  if line:find(RESPONSE_AUTH_TOO_MANY, 1, true) then
    return true
  end

  if line:find(RESPONSE_WELCOME, 1, true) then
    return true
  end

  if line:find("^Server IP:", 1) then
    return true
  end

  if line:find("^Server port:", 1) then
    return true
  end

  if line:find("^Max players:", 1) then
    return true
  end

  if line:find("^Game mode:", 1) then
    return true
  end

  if line:find("^Game name:", 1) then
    return true
  end

  if line:find(RESPONSE_INF_PREFIX, 1, true) then
    return true
  end

  return false
end

local function send_line(line)
  gsw.send(line .. CRLF)
end

local function send_pending_command()
  if authentication_state == AuthenticationState.AUTHORIZED and #pending_commands > 0 then
    local cmd = table.remove(pending_commands, 1)
    awaiting_command_response = true
    send_line(cmd)
  end
end

local function mark_authenticated()
  authentication_state = AuthenticationState.AUTHORIZED
  gsw.authenticated()
  send_pending_command()
end

local function send_password()
  if stored_password == "" then
    error("AuthError: password required")
  end

  authentication_state = AuthenticationState.AWAITING
  send_line(stored_password)
end

---@param line string
local function process_auth_line(line)
  if line:find(RESPONSE_AUTH_TOO_MANY, 1, true) then
    error("AuthError: too many failed login attempts")
  end

  if line:find(RESPONSE_AUTH_FAIL, 1, true) then
    error("AuthError: authentication failed")
  end

  if line:find(RESPONSE_AUTH_SUCCESS, 1, true) then
    mark_authenticated()
  end
end

---@param line string
local function process_command_line(line)
  if not awaiting_command_response then
    return
  end

  if is_login_or_auth_line(line) then
    return
  end

  gsw.response(line)
end

---@param line string
local function process_none_line(line)
  if line:find(PROMPT_PASSWORD, 1, true) then
    send_password()
    return
  end

  if stored_password == "" then
    mark_authenticated()
  end
end

---@param line string
local function process_line(line)
  switch (authentication_state) {
    [AuthenticationState.NONE] = function()
      process_none_line(line)
    end,
    [AuthenticationState.AWAITING] = function()
      process_auth_line(line)
    end,
    [AuthenticationState.AUTHORIZED] = function()
      process_command_line(line)
    end,
  }
end

function on_connected()
  -- Wait for the server password prompt in process(); nothing to send yet.
end

---@param cmd string
---@param params { password: string }
function send_command(cmd, params)
  stored_password = params.password or ""
  table.insert(pending_commands, cmd)

  if authentication_state == AuthenticationState.AUTHORIZED then
    send_pending_command()
  end
end

---@param data Buffer
function process(data)
  local text = telnet_process(data)

  if text ~= "" then
    line_buffer:append(text)
    line_buffer:consume(process_line)
  end
end
