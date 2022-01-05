import '../lib/polyfill'
import { DataWriter } from '../lib/data-writer'
import { InvalidResponseError } from '../lib/response-error'
import { parseQuakeInfo } from './quake'

export const info: ProtocolInfo = {
  id: 'quake3',
  name: 'Quake 3',
  feature: 'query',
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

const extractServerInfo = (inf: Info, plist: Player[]): ServerInfo => ({
  [InfoField.SERVER_NAME]: String(inf.sv_hostname),
  [InfoField.GAME_NAME]: String(inf.gamename),
  [InfoField.GAME_VERSION]: String(inf.version || inf.shortversion),
  [InfoField.MAP]: String(inf.mapname),
  [InfoField.NUM_PLAYERS]: plist.length,
  [InfoField.MAX_PLAYERS]: Number(inf.sv_maxclients),
  [InfoField.PRIVATE]: Boolean(inf.g_needpass),
})

const parsePlayer = (str: string): Player => {
  const parts = str
    .split('\"')
    .flatMap((part, index) => index % 2 ? part : part.split(' '))
    .filter(Boolean)

  return Object.assign<Player, string[]>({}, parts)
}

export const processResponse: ProcessResponseFn = data => {
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
  const players = parts.slice(2).map(parsePlayer)
  const inf = extractServerInfo(all_info, players)

  gsw.details(all_info)
  gsw.sinfo(inf)
  gsw.plist(players)
}
