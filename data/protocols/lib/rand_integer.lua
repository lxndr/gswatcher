---@meta

---@param min integer
---@param max integer
local function rand_integer(min, max)
  local rnd = math.random()
  return math.floor(rnd * (max - min)) + min
end

return rand_integer
