-- protocol = {
--   id        = "quake",
--   name      = "Quake",
--   feature   = "query",
--   transport = "udp",
-- }

-- function query()
-- end

local function parse_info(data)
  local values = {}
  local pos = 1

  while true do
    local s, e, key, val = data:find("^\\([%w_]+)\\([^\\]*)", pos)

    if not key then
        break
    end

    values[key] = val
    pos = e + 1
  end

  return values
end

-- function process(data)
--   error("not implemented")
-- end

return {
  parse_info = parse_info,
}
