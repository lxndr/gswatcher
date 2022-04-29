local truthy_values = {
  [true]   = true,
  [1]      = true,
  ["1"]    = true,
  ["true"] = true,
  ["yes"]  = true,
  ["on"]   = true,
}

local function to_boolean(val)
  return truthy_values[val] or false
end

return to_boolean
