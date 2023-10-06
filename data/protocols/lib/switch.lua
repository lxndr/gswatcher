local function switch(val)
  return function (cases)
    local fn = cases[val] or cases.default

    if fn then
      fn()
    end
  end
end

return switch
