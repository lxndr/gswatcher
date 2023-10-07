local DataWriter = require("lib/DataWriter")
local quake2 = require("quake2")

protocol = {
  id        = "quake3",
  name      = "Quake 3",
  feature   = "query",
  transport = "udp",
}

function query()
  local w = DataWriter()
  w:i32le(-1)
  w:string("getstatus\n")
  gsw.send(w.buf)
end

function process(data)
  quake2.process_response(data, -1, "statusResponse")
end
