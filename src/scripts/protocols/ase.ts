import { DataReader } from '../lib/data-reader'
import { InvalidResponseError } from '../lib/response-error'
import { str2bool } from '../lib/str2bool'

interface ASEInfo extends Info {
  gameName: string
  port: string
  serverName: string
  gameType: string
  map: string
  version: string
  private: string
  numPlayers: string
  maxPlayers: string
}

export const info: ProtocolInfo = {
  id: 'ase',
  name: 'All-Seeing Eye',
  feature: 'query',
  transport: 'udp',
}

export const query: QueryFn = () =>
  gsw.send(new Buffer('s'))

const readString = (r: DataReader) => {
  const len = r.u8()

  if (len == 0) {
    throw new InvalidResponseError('string must exist')
  }

  return r.lstring(len - 1)
}

const readGeneralInfo = (r: DataReader): ASEInfo => ({
  gameName: readString(r),
  port: readString(r),
  serverName: readString(r),
  gameType: readString(r),
  map: readString(r),
  version: readString(r),
  private: readString(r),
  numPlayers: readString(r),
  maxPlayers: readString(r),
})

const readKeyValues = (r: DataReader) => {
  const inf: Info = {}

  while (!r.is_end) {
    const key = readString(r)

    if (!key) {
      break;
    }

    const val = readString(r)
    inf[key] = val
  }

  return inf
}

const readPlayerList = (r: DataReader) => {
  let pfields: PlayerField[] = [{
    title: 'Name',
    kind: 'string',
    field: 'name',
    main: true,
  }]

  const players: Player[] = []

  while (!r.is_end) {
    const player: Player = {}
    const flags = r.u8()

    if (flags & 0x01) {
      player.name = readString(r)
    }

    if (flags & 0x02) {
      player.team = readString(r)
    }

    if (flags & 0x04) {
      player.skin = readString(r)
    }

    if (flags & 0x08) {
      player.score = readString(r)
    }

    if (flags & 0x10) {
      player.ping = readString(r)
    }

    if (flags & 0x20) {
      player.time = readString(r)
    }

    players.push(player)
  }

  if (players.length) {
    pfields = Object.keys(players[0]).map(key => ({
      title: key.slice(0, 1).toLocaleUpperCase() + key.slice(1),
      kind: 'string',
      field: key,
      main: key === 'name',
    }))
  }

  return { pfields, players }
}

const normalizeServerInfo = (inf: ASEInfo): ServerInfo => ({
  [InfoField.SERVER_NAME]: inf.serverName,
  [InfoField.GAME_NAME]: inf.gameName,
  [InfoField.GAME_VERSION]: inf.version,
  [InfoField.GAME_MODE]: inf.gameType,
  [InfoField.MAP]: inf.map,
  [InfoField.PRIVATE]: str2bool(inf.private),
  [InfoField.NUM_PLAYERS]: Number(inf.numPlayers),
  [InfoField.MAX_PLAYERS]: Number(inf.maxPlayers),
})

export const processResponse: ProcessResponseFn = data => {
  const r = new DataReader(data)

  const sig = r.lstring(4)

  if (sig !== 'EYE1') {
    throw new InvalidResponseError('not an All-Seeing Eye response')
  }

  const generalInfo = readGeneralInfo(r)
  const keyValues = readKeyValues(r)
  const { pfields, players } = readPlayerList(r)

  const details = { ...generalInfo, ...keyValues }
  const inf = normalizeServerInfo(details)

  gsw.sinfo(details, inf)
  gsw.plist(pfields, players)
}
