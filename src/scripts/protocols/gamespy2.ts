import { InvalidResponseError } from '../lib/response-error'
import { DataReader } from '../lib/data-reader'
import { DataWriter } from '../lib/data-writer'
import { genRandomInteger } from '../lib/gen-random-integer'

export const info: ProtocolInfo = {
  id: 'gamespy2',
  name: 'GameSpy 2',
  feature: 'query',
  transport: 'udp',
}

let requestID = 0

export const query: QueryFn = () => {
  requestID = genRandomInteger(0, 256 ** 4)

  const data = new DataWriter()
    .u8(0xfe).u8(0xfd) // header
    .u8(0x00)          // delimiter
    .u32le(requestID)  // request id
    .u8(0xff)          // get server info and rules
    .u8(0xff)          // get player info
    .u8(0xff)          // get team info
    .buf

  gsw.send(data)
}

const normalizeServerInfo = (inf: Info): ServerInfo => ({
  [InfoField.SERVER_NAME]: String(inf.hostname),
  [InfoField.NUM_PLAYERS]: Number(inf.numplayers),
  [InfoField.MAX_PLAYERS]: Number(inf.maxplayers),
  [InfoField.MAP]: String(inf.maptitle || inf.mapname),
  [InfoField.GAME_VERSION]: String(inf.gamever),
  [InfoField.SECURE]: inf.sv_punkbuster === '1',
})

const parseServerInfo = (r: DataReader, delimiter: number): Info => {
  const inf: Info = {}

  while (!r.is_end) {
    const key = r.zstring(delimiter)
    const val = r.zstring(delimiter)

    if (!(key && val)) {
      break;
    }

    inf[key] = val
  }

  return inf
}

const readPlayerFields = (r: DataReader, delimiter: number) => {
  const fields: PlayerField[] = []

  while (!r.is_end) {
    const field = r.zstring(delimiter)

    if (!field) {
      break;
    }

    fields.push({
      title: field.slice(0, 1).toLocaleUpperCase() + field.slice(1),
      kind: 'string',
      field: field,
      main: field === 'name',
    })
  }

  return fields
}

const parsePlayerList = (r: DataReader, delimiter: number) => {
  const players: Player[] = []
  const player_count = r.u8()
  const pfields = readPlayerFields(r, delimiter)
  const field_count = pfields.length

  for (let iplayer = 0; iplayer < player_count && !r.is_end; iplayer++) {
    const player: Player = {}

    for (let ifield = 0; ifield < field_count && !r.is_end; ifield++) {
      const val = r.zstring(delimiter)
      player[pfields[ifield].field] = val
    }

    players.push(player)
  }

  return { pfields, players }
}

export const processResponse: ProcessResponseFn = data => {
  const r = new DataReader(data)

  const delimiter = r.u8()
  const reqid = r.u32le()

  if (reqid !== requestID) {
    throw new InvalidResponseError('invalid request id')
  }

  const details = parseServerInfo(r, delimiter)
  const { pfields, players } = parsePlayerList(r, delimiter)
  const inf = normalizeServerInfo(details)

  gsw.sinfo(details, inf)
  gsw.plist(pfields, players)
}
