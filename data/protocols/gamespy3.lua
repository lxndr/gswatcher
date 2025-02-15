-- Game Server Watcher
-- Copyright (C) 2025  Alexander Burobin

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

local switch = require("lib/switch")
local rand_integer = require("lib/rand_integer")
local DataReader = require("lib/DataReader")
local DataWriter = require("lib/DataWriter")
local CompoundResponse = require("lib/CompoundResponse")
local gamespy = require("lib/gamespy")

protocol = {
  id        = "gamespy3",
  name      = "GameSpy 3",
  feature   = "query",
  transport = "udp",
}

---@type CompoundResponse
local response

---@param request_id integer
local function send_stat_packet(request_id)
  local w = DataWriter()
    :u8(0xfe):u8(0xfd)
    :u8(0)
    :i32be(request_id)
    :u8(0xff)          -- get server info and rules
    :u8(0xff)          -- get player info
    :u8(0xff)          -- get team info
    :u8(0x01)

  gsw.send(w.buf)
end

function query()
  local request_id = rand_integer(1, 0x7fffffff)

  response = CompoundResponse(request_id)
  response.details = {}
  response.pfields = {}
  response.plist = {}

  send_stat_packet(request_id)
end

---@param data string
function process(data)
  local r = DataReader(data)
  local response_type = r:u8()
  local response_id = r:u32be()

  if response_id ~= response.reqid then
    error("invalid response: invalid session id " .. response_id)
  end

  switch (response_type) {
    [0] = function()
      local details, sinfo, pfields, plist = gamespy.parse_v3_stats_response(r, response, response_id)

      if details ~= nil then
        gsw.sinfo(details, sinfo)
        gsw.plist(pfields, plist)
      end
    end,
    default = function()
      error("invalid response: invalid response type " .. response_type)
    end,
  }
end
