import { DataReader } from './lib/data-reader'
import { DataWriter } from './lib/data-writer'
import { InvalidResponseError } from './lib/response-error'

export const info: ProtocolInfo = {
  id: 'quake3',
  name: 'Quake 3',
  transport: 'udp',
}

const createPacket = (parts: string[]) => {
  const data = parts.map(part => `${part}\n`).join('')
  const w = new DataWriter()
  w.i32le(-1)
  w.string(data)
  return w.buf
}

export const query = () =>
  gsw.send(createPacket(['getstatus']))

const parseInfo = (str: string): ServerInfo => {
  const parts = str.substr(1).split('\\')

  const obj: Record<string, string> = {}

  for (let i = 0; i < parts.length; i++) {
    const key = parts[i * 2]
    const val = parts[i * 2 + 1]
    obj[key] = val
  }

  const inf: ServerInfo = {}

  if ('sv_hostname' in obj) {
    inf.name = obj.sv_hostname
  }

  if ('sv_maxclients' in obj) {
    inf.max_players = parseInt(obj.sv_maxclients, 10)
  }

  if ('version' in obj) {
    inf.version = obj.version
  }

  if ('mapname' in obj) {
    inf.map = obj.mapname
  }

  return inf
}

const parsePlayer = (str: string): Player[] => {
  return []
}

export const processResponse = (data: Buffer) => {
  const hdr = data.readInt32LE()

  if (hdr !== -1) {
    throw new InvalidResponseError('invalid header')
  }

  const str = data.toString('binary', 4)
  const parts = str.split('\n').filter(Boolean)

  if (parts[0] !== 'statusResponse') {
    throw new InvalidResponseError('invalid packet')
  }

  if (!parts[1].startsWith('\\')) {
    throw new InvalidResponseError('invalid packet')
  }

  const inf = parseInfo(parts[1])
  const players = parts.slice(2).map(parsePlayer)

  gsw.sinfo(inf)
}
