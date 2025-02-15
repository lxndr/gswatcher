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

-- https://wiki.unrealadmin.org/UT3_query_protocol
-- https://wiki.vg/Query

local switch = require("lib/switch")
local rand_integer = require("lib/rand_integer")
local DataReader = require("lib/DataReader")
local CompoundResponse = require("lib/CompoundResponse")
local gamespy = require("lib/gamespy")

protocol = {
  id        = "gamespy4",
  name      = "GameSpy 4",
  feature   = "query",
  transport = "udp",
}

---@type CompoundResponse
local response

---@param session_id integer
local function send_challenge_packet(session_id)
  local data = gamespy.create_challenge_request(session_id)
  gsw.send(data)
end

---@param session_id integer
---@param challenge integer
local function send_stats_packet(session_id, challenge)
  local data = gamespy.create_stats_request(session_id, challenge)
  gsw.send(data)
end

function query()
  local request_id = rand_integer(1, 0x7fffffff)
  response = CompoundResponse(request_id)
  response.details = {}
  response.pfields = {}
  response.plist = {}

  send_challenge_packet(request_id)
end

---@param r DataReader
local function parse_challenge_response(r)
  local str = r:zstring()
  local challenge = tonumber(str, 10)
  if math.type(challenge) ~= "integer" then
    error("invalid response: recieved challenge is not integer")
  end

  return challenge
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
    [9] = function()
      local challenge = parse_challenge_response(r)
      send_stats_packet(response.reqid, challenge)
    end,
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
