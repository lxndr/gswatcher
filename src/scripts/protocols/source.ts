import 'core-js/features/array/find'
import jsbi from 'jsbi'
import { DataReader } from './lib/data-reader'
import { DataWriter } from './lib/data-writer'

interface PacketHeader {
  reqid: number
  number: number
  total: number
  size?: number
  compressed?: boolean
  dataSize?: number
  crc32?: number
  payload: Buffer
}

interface ServerInfoExended extends ServerInfo {
  appid?: number
  protocol_version: number
  dir: string
  desc: string
  num_bots: number
  address?: string
  keywords: string[]
  server_port?: number
  steamid?: string
  sourcetv_port?: number
  sourcetv_name?: string
  theShip?: {
    mode: number
    witnesses: number
    duration: number
  }
  mod?: {
    link: string
    download: string
    null: number
    version: number
    size: number
  }
}

interface Request {
  packets: Buffer[]
  firstPacketTime: number
  total: number
}

export const info: ProtocolInfo = {
  id: 'source',
  name: 'Source Engine',
  transport: 'udp',
}

const requests: Record<number, Request> = {}

let gotChallenge = -1
let gotServerInfo: ServerInfoExended | null = null
let gotPlayerList: (Player[] | null) = null

const sendPacket = (type: string, payload?: Buffer) => {
  const w = new DataWriter()

  w.i32le(-1)
  w.lstring(type, 1)

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

const getGameMode = (inf: ServerInfoExended): (string | null) => {
  switch (inf.appid) {
    case 500: { // Left 4 Dead
      const m = inf.desc.match(/^L4D - ([a-z-]+)/i)
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

      const mode = inf.keywords.find(keyword => keyword in modes)
      return mode ? modes[mode] : null
    }
    case 730: { // CS: GO
      const modes: Record<string, string> = {
        competitive: 'Competitive',
      }

      const mode = inf.keywords.find(keyword => keyword in modes)
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
  const inf: ServerInfoExended = {
    address: r.zstring(),
    name: r.zstring(),
    map: r.zstring(),
    dir: r.zstring(),
    desc: r.zstring(),
    num_players: r.u8(),
    max_players: r.u8(),
    num_bots: 0,
    protocol_version: r.u8(),
    server_type: r.lstring(1),
    os: r.lstring(1),
    has_password: Boolean(r.u8()),
    keywords: [],
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

  inf.secure = Boolean(r.u8())
  inf.num_bots = r.u8()

  return inf
}

const readServerInfo = (r: DataReader) => {
  const inf: ServerInfoExended = {
    protocol_version: r.u8(),
    name: r.zstring(),
    map: r.zstring(),
    dir: r.zstring(),
    desc: r.zstring(),
    appid: r.u16le(),
    num_players: r.u8(),
    max_players: r.u8(),
    num_bots: r.u8(),
    server_type: r.lstring(1),
    os: r.lstring(1),
    has_password: Boolean(r.u8()),
    secure: Boolean(r.u8()),
    keywords: [],
  }

  if (inf.appid === 2400) { // The Ship
    inf.theShip = {
      mode: r.u8(),
      witnesses: r.u8(),
      duration: r.u8(),
    }
  }

  inf.version = r.zstring()
  const edf = r.is_end() ? 0 : r.u8()

  if (edf & 0x80) {
    inf.server_port = r.u16le()
  }

  if (edf & 0x10) {
    // unneeded field, skip it for performance's sake
    // inf.steamid = String(r.u64le())
    r.skip(8)
  }

  if (edf & 0x40) {
    inf.sourcetv_port = r.u16le()
    inf.sourcetv_name = r.zstring()
  }

  if (edf & 0x20) {
    inf.keywords = r.zstring().split(',').filter(Boolean);
  }

  if (edf & 0x01) {
    const gameId = r.u64le()
    const appid = jsbi.bitwiseAnd(gameId, jsbi.BigInt(0xffffff))
    inf.appid = jsbi.toNumber(appid)
  }

  const mode = getGameMode (inf)

  if (mode) {
    inf.game_mode = mode
  }

  return inf
}

const readPlayerList = (r: DataReader, inf: ServerInfoExended | null) => {
  const players: Player[] = []
  const count = r.u8()

  for (let i = 0; i < count; i++) {
    const player: Player = {
      index: r.u8(),
      name: r.zstring(),
      score: r.u32le(),
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
    case 'I':
      gotServerInfo = readServerInfo(r)
      gotChallenge = -1
      gsw.sinfo(gotServerInfo)
      nextQuery ()
      break
    case 'm':
      gotServerInfo = readServerInfoGold(r)
      gotChallenge = -1
      gsw.sinfo(gotServerInfo)
      nextQuery ()
      break
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

const readHeaderGold = (r: DataReader): PacketHeader => {
  const reqid = r.u32le()
  let number = r.u8()
  const total = number & 0x0f
  number >>= 4

  if (!reqid) {
    throw new Error('Incorrect reqid')
  }

  if (!total) {
    throw new Error('Incorrect total')
  }

  if (number >= total) {
    throw new Error('Incorrect number')
  }

  const payload = r.data()
  return { reqid, total, number, payload }
}

const readHeader = (r: DataReader): PacketHeader => {
  const reqid = r.u32le()
  const total = r.u8()
  const number = r.u8()
  const size = r.u16le()
  const compressed = Boolean(reqid >> 31)
  let dataSize, crc32

  if (!reqid) {
    throw new Error('Incorrect reqid')
  }

  if (!total) {
    throw new Error('Incorrect total')
  }

  if (number >= total) {
    throw new Error('Incorrect number')
  }

  if (number === 0 && compressed) {
    dataSize = r.u32le()
    crc32 = r.u32le()
  }

  const payload = r.data()
  return { reqid, total, number, compressed, dataSize, crc32, payload }
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

const storePacket = (pak: PacketHeader) => {
  if (!requests[pak.reqid]) {
    requests[pak.reqid] = {
      packets: [],
      firstPacketTime: Date.now(),
      total: pak.total,
    }
  }

  const req = requests[pak.reqid]

  if (req.total !== pak.total) {
    throw new Error('Incorrect packet')
  }

  req.packets[pak.number] = pak.payload
  return req
}

const gotAllPackets = (req: Request) =>
  req.packets.every(pak => pak)

export const processResponse = (data: Buffer) => {
  const r = new DataReader(data)
  const format = r.i32le()

  if (format === -1) {
    readPayload(r)
  } else if (format == -2) {
    const pak = tryReadHeader(r)

    if (!pak) {
      throw new Error('Could not read packet header response')
    }

    if (pak.compressed) {
      throw new Error('Compressed split packets are not supported')
    }

    const req = storePacket(pak)

    if (gotAllPackets(req)) {
      const data = Buffer.concat(req.packets)
      const r = new DataReader(data)
      readPayload(r)
      delete requests[pak.reqid]
    }
  }
}
