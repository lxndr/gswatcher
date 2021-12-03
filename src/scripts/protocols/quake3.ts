import { DataWriter } from './lib/data-writer'
import { InvalidResponseError } from './lib/response-error'
import { parseQuakeInfo } from './quake'

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

const extractServerInfo = (dict: Info): ServerInfo => {
  return {
    [InfoField.SERVER_NAME]: String(dict.sv_hostname),
    [InfoField.GAME_NAME]: String(dict.gamename),
    [InfoField.GAME_VERSION]: String(dict.version),
    [InfoField.MAP]: String(dict.mapname),
    [InfoField.MAX_PLAYERS]: Number(dict.sv_maxclients),
    [InfoField.PRIVATE]: Boolean(dict.g_needpass),
  }
}

const parsePlayer = (str: string): Player => {
  const parts = str
    .split('\"')
    .flatMap((part, index) => index % 2 ? part : part.split(' '))
    .filter(Boolean)

  return Object.assign<Player, string[]>({}, parts)
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

  const all_info = parseQuakeInfo(parts[1])
  gsw.details(all_info)

  const inf = extractServerInfo(all_info)
  gsw.sinfo(inf)

  const players = parts.slice(2).map(parsePlayer)
  gsw.plist(players)
}
