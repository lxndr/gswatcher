local function concat_tables(...)
  local args = {...}
  local res = {}

  for _, t in pairs(args) do
    for k, v in pairs(t) do
      res[k] = v
    end
  end

  return res
end

return concat_tables
