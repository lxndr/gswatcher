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

---@param field_id string
local function add_player_field(field_id)
  for _, field in ipairs(response.pfields) do
    if field.field == field_id then
      return
    end
  end

  table.insert(response.pfields, {
    title = field_id:sub(1, 1):upper() .. field_id:sub(2, -2),
    kind = "string",
    field = field_id,
  })
end

---@param r DataReader
---@param response_id integer
local function parse_stat_response(r, response_id)
  local splitnum = r:zstring()

  if splitnum ~= "splitnum" then
    error("invalid response: invalid 'splitnum'")
  end

  local num_packets = r:u8()
  local final = (num_packets & 0x80) > 0
  local number = (num_packets & 0x7f) + 1

  local packet = {
    reqid = response_id,
    number = number,
    data = {}
  }

  if final then
    packet.total = number
  end

  while not r:is_end() do
    local info_type = r:u8()

    switch (info_type) {
      -- server info
      [0] = function()
        while not r:is_end() do
          local key = r:zstring()

          if #key == 0 then
            break
          end

          local val = r:zstring()
          response.details[key] = val
        end
      end,
      -- player info
      [1] = function()
        while not r:is_end() do
          local field = r:zstring()

          if #field == 0 then
            break
          end

          add_player_field(field)
          local idx = r:u8() + 1

          while not r:is_end() do
            local val = r:zstring()

            if #val == 0 then
              break
            end

            if response.plist[idx] == nil then
              response.plist[idx] = {}
            end

            response.plist[idx][field] = val
            idx = idx + 1
          end
        end
      end,
      -- team info
      [2] = function()
        while not r:is_end() do
          local field = r:zstring()

          if #field == 0 then
            break
          end

          local idx = r:u8() + 1

          while not r:is_end() do
            local val = r:zstring()

            if #val == 0 then
              break
            end

            -- print("Team info: " .. field .. " = " .. val)
          end
        end
      end,
      default = function()
        error("invalid response: invalid info type " .. info_type)
      end,
    }
  end

  response:add_packet(packet)

  if response:got_all_packets() then
    local inf = gamespy.extract_server_info(response.details)
    gsw.sinfo(response.details, inf)
    print(#response.pfields)
    gsw.plist(response.pfields, response.plist)
  end
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
      parse_stat_response(r, response_id)
    end,
    default = function()
      error("invalid response: invalid response type " .. response_type)
    end,
  }
end
