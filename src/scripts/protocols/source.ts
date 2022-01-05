// https://developer.valvesoftware.com/wiki/Server_Queries

import '../lib/polyfill'
// import jsbi from 'jsbi'
import { DataReader } from '../lib/data-reader'
import { DataWriter } from '../lib/data-writer'
import { CompoundResponse, Packet, ResponseStore } from '../lib/response-store'
import { InvalidResponseError } from '../lib/response-error'

export const info: ProtocolInfo = {
  id: 'source',
  name: 'Source Engine',
  feature: 'query',
  transport: 'udp',
}

interface SourcePlayer extends Player {
  index: number
  name: string
  score: number
  duration: number
  deaths?: number
  money?: number
}

interface SourcePacket extends Packet<Buffer> {
  total: number
  size?: number
  compressed?: boolean
  dataSize?: number
  crc32?: number
}

class SourceResponse extends CompoundResponse<Buffer, SourcePacket> {
  combine() {
    return Buffer.concat(this.packets.map(packet => packet.data))
  }
}

class SourceResponseStore extends ResponseStore<Buffer, SourcePacket, SourceResponse> {
  constructor() {
    super(SourceResponse)
  }

  addPacket(packet: SourcePacket) {
    const { reqid, total, compressed } = packet

    if (compressed) {
      throw new Error('Compressed split packets are not supported')
    }

    const res = this.ensureResponse(reqid)

    if (res.total) {
      if (res.total !== total) {
        throw new InvalidResponseError('incorrect packet')
      }
    }

    res.total = total
    return super.addPacket(packet)
  }
}

interface SourceServerDetails extends Info {
  protocolVersion: number
  name: string
  map: string
  folder: string
  game: string
  appid?: number
  numPlayers: number
  maxPlayers: number
  numBots: number
  serverType: string
  environment: string
  visibility: number
  vac: number
  theShip?: {
    mode: number
    witnesses: number
    duration: number
  }
  version?: string
  port?: number
  steamid?: string
  sourcetv?: {
    port: number
    name: string
  }
  keywords?: string
  gameid?: string

  // GoldSource
  address?: string
  mod?: {
    link: string
    download: string
    null: number
    version: number
    size: number
  }
}

const responses = new SourceResponseStore()

let gotChallenge = -1
let gotServerInfo: SourceServerDetails | null = null
let gotPlayerList: (Player[] | null) = null

const sendPacket = (type: string, payload?: Buffer) => {
  const w = new DataWriter()

  w.i32le(-1)
  w.string(type, 1)

  if (payload) {
    w.data(payload)
  }

  gsw.send(w.buf)
}

const sendServerInfoPacket = (challenge?: number) => {
  const w = new DataWriter()
  w.zstring('Source Engine Query')

  if (challenge) {
    w.i32le(challenge)
  }

  sendPacket('T', w.buf)
}

const sendPlayerListPacket = (challenge?: number) => {
  const w = new DataWriter()

  if (challenge) {
    w.i32le(challenge)
  }

  sendPacket('U', w.buf)
}

export const nextQuery = () => {
  if (!gotServerInfo) {
    sendServerInfoPacket(gotChallenge)
    return
  }

  if (!gotPlayerList) {
    sendPlayerListPacket(gotChallenge)
  }
}

export const query = () => {
  gotChallenge = -1
  gotServerInfo = null
  gotPlayerList = null
  nextQuery ()
}

const getGameMode = (inf: SourceServerDetails): (string | null) => {
  switch (inf.appid) {
    case 500: { // Left 4 Dead
      const m = inf.game.match(/^L4D - ([a-z-]+)/i)
      return m?.[1] || null
    }
    case 550: { // Left 4 Dead 2
      const modes: Record<string, string> = {
        coop: 'Co-op',
        realism: 'Realism',
        survival: 'Survival',
        versus: 'Versus',
        scavenge: 'Scavenge',
      }

      const mode = inf.keywords?.split(',').find(keyword => keyword in modes)
      return mode ? modes[mode] : null
    }
    case 730: { // CS: GO
      const modes: Record<string, string> = {
        competitive: 'Competitive',
      }

      const mode = inf.keywords?.split(',').find(keyword => keyword in modes)
      return mode ? modes[mode] : null
    }
    case 2400: { // The Ship
      const modes = [
        'Hunt',
        'Elimination',
        'Duel',
        'Deathmatch',
        'VIP Team',
        'Team Elimination',
      ]

      return modes[inf.theShip?.mode!] || null
    }
  }

  return null
}

const readServerInfoGold = (r: DataReader) => {
  const inf: SourceServerDetails = {
    address: r.zstring(),
    name: r.zstring(),
    map: r.zstring(),
    folder: r.zstring(),
    game: r.zstring(),
    numPlayers: r.u8(),
    maxPlayers: r.u8(),
    numBots: 0,
    protocolVersion: r.u8(),
    serverType: r.lstring(1),
    environment: r.lstring(1),
    visibility: r.u8(),
    vac: 0,
  }

  const mod = r.u8()

  if (mod) {
    inf.mod = {
      link: r.zstring(),
      download: r.zstring(),
      null: r.u8(),
      version: r.i32le(),
      size: r.i32le(),
    }
  }

  inf.vac = r.u8()
  inf.numBots = r.u8()

  return inf
}

const readServerInfo = (r: DataReader) => {
  const inf: SourceServerDetails = {
    protocolVersion: r.u8(),
    name: r.zstring(),
    map: r.zstring(),
    folder: r.zstring(),
    game: r.zstring(),
    appid: r.u16le(),
    numPlayers: r.u8(),
    maxPlayers: r.u8(),
    numBots: r.u8(),
    serverType: r.lstring(1),
    environment: r.lstring(1),
    visibility: r.u8(),
    vac: r.u8(),
  }

  if (inf.appid === 2400) { // The Ship
    inf.theShip = {
      mode: r.u8(),
      witnesses: r.u8(),
      duration: r.u8(),
    }
  }

  inf.version = r.zstring()
  const edf = r.is_end ? 0 : r.u8()

  if (edf & 0x80) {
    inf.port = r.u16le()
  }

  if (edf & 0x10) {
    inf.steamid = String(r.u64le())
  }

  if (edf & 0x40) {
    inf.sourcetv = {
      port: r.u16le(),
      name: r.zstring(),
    }
  }

  if (edf & 0x20) {
    inf.keywords = r.zstring();
  }

  if (edf & 0x01) {
    inf.gameid = String(r.u64le())
    // const gameId = r.u64le()
    // const appid = jsbi.bitwiseAnd(gameId, jsbi.BigInt(0xffffff))
    // inf.appid = jsbi.toNumber(appid)
  }

  return inf
}

const normalizeServerInfo = (inf: SourceServerDetails): ServerInfo => ({
  [InfoField.GAME_MODE]: getGameMode(inf) || undefined,
  [InfoField.GAME_VERSION]: inf.version,
  [InfoField.SERVER_NAME]: inf.name,
  [InfoField.SERVER_TYPE]: inf.serverType,
  [InfoField.SERVER_OS]: inf.environment,
  [InfoField.MAP]: inf.map,
  [InfoField.NUM_PLAYERS]: inf.numPlayers,
  [InfoField.MAX_PLAYERS]: inf.maxPlayers,
  [InfoField.PRIVATE]: Boolean(inf.visibility),
  [InfoField.SECURE]: Boolean(inf.vac),
})

const readPlayerList = (r: DataReader, inf: SourceServerDetails | null) => {
  const players: SourcePlayer[] = []
  const count = r.u8()

  for (let i = 0; i < count; i++) {
    const player: SourcePlayer = {
      index: r.u8(),
      name: r.zstring(),
      score: r.i32le(),
      duration: r.f32le(),
    }

    if (inf && inf.appid == 2400) { // The Ship
      player.deaths = r.u32le()
      player.money = r.u32le()
    }

    players.push(player)
  }

  return players
}

const readChallenge = (r: DataReader) =>
  r.i32le()

const readPayload = (r: DataReader) => {
  const type = r.lstring(1)

  switch (type) {
    case 'I': {
      gotChallenge = -1
      gotServerInfo = readServerInfo(r)
      gsw.details(gotServerInfo)
      const normalizedServerInfo = normalizeServerInfo(gotServerInfo)
      gsw.sinfo(normalizedServerInfo)
      nextQuery ()
      break
    }
    case 'm': {
      gotChallenge = -1
      gotServerInfo = readServerInfoGold(r)
      gsw.details(gotServerInfo)
      const normalizedServerInfo = normalizeServerInfo(gotServerInfo)
      gsw.sinfo(normalizedServerInfo)
      nextQuery ()
      break
    }
    case 'D':
      gotPlayerList = readPlayerList(r, gotServerInfo)
      gotChallenge = -1
      gsw.plist(gotPlayerList)
      nextQuery ()
      break
    case 'A':
      gotChallenge = readChallenge(r)
      nextQuery ()
  }
}

const readHeaderGold = (r: DataReader): SourcePacket => {
  const reqid = r.u32le()
  let number = r.u8()
  const total = number & 0x0f
  number >>= 4

  if (!reqid) {
    throw new InvalidResponseError(`request id cannot be ${reqid}`)
  }

  if (!total) {
    throw new InvalidResponseError(`total packets cannot be ${total}`)
  }

  if (number >= total) {
    throw new InvalidResponseError(`packet number ${number} cannot be greater or equal ${total}`)
  }

  const data = r.data()
  return { reqid, total, number, data }
}

const readHeader = (r: DataReader): SourcePacket => {
  const reqid = r.u32le()
  const total = r.u8()
  const number = r.u8()
  const size = r.u16le()
  const compressed = Boolean(reqid >> 31)
  let dataSize, crc32

  if (!reqid) {
    throw new InvalidResponseError(`request id cannot be ${reqid}`)
  }

  if (!total) {
    throw new InvalidResponseError(`total packets cannot be ${total}`)
  }

  if (number >= total) {
    throw new InvalidResponseError(`packet number ${number} cannot be greater or equal ${total}`)
  }

  if (number === 0 && compressed) {
    dataSize = r.u32le()
    crc32 = r.u32le()
  }

  const data = r.data()
  return { reqid, total, number, compressed, dataSize, crc32, data }
}

const tryReadHeader = (r: DataReader) => {
  const pos = r.pos
  let pak = readHeaderGold(r)

  if (!pak) {
    r.pos = pos
    pak = readHeader(r)
  }

  return pak
}

export const processResponse: ProcessResponseFn = data => {
  const r = new DataReader(data)
  const format = r.i32le()

  if (format === -1) {
    readPayload(r)
  } else if (format == -2) {
    const pak = tryReadHeader(r)

    const res = responses.addPacket(pak)

    if (res.gotAllPackets()) {
      const data = res.combine()
      responses.remove(pak.reqid)

      const r = new DataReader(data)
      readPayload(r)
    }
  }
}
