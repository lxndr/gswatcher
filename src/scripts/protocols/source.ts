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
  payload: Uint8Array
}

interface ServerInfoExended extends ServerInfo {
  appid?: number
  protocolVersion: number
  dir: string
  desc: string
  numBots: number
  address?: string
  keywords?: string
  serverPort?: number
  steamid?: string
  sourcetvPort?: number
  sourcetvName?: string
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

type GameDef = Partial<ServerInfo> | ((inf: ServerInfoExended) => Partial<ServerInfo>)

interface Request {
  packets: Uint8Array[]
  firstPacketTime: number
  total: number
}

export const info: ProtocolInfo = {
  name: 'Source Engine',
  version: '1.0',
  transport: 'udp',
}

const requests: Record<number, Request> = {}

let gotChallenge = -1
let gotServerInfo: ServerInfoExended | null = null
let gotPlayerList: (Player[] | null) = null

const games: Record<number, GameDef> = {
  0: inf => {
    if (inf.dir === 'cstrike') {
      return {
        gameId: 'cs',
        gameName: 'Counter-Strike',
      }
    } else {
      return {}
    }
  },
  10: inf => {
    if (inf.dir === 'cstrike') {
      return {
        gameId: 'cs',
        gameName: 'Counter-Strike',
      }
    } else {
      return {
        gameId: 'hl',
        gameName: 'Half-Life',
      }
    }
  },
  20: {
    gameId: 'tf',
    gameName: 'Team Fortress',
  },
  30: {
    gameId: 'dod',
    gameName: 'Day of Defeat',
  },
  80: {
    gameId: 'czero',
    gameName: 'Counter-Strike: Condition Zero',
  },
  240: {
    gameId:'css',
    gameName: 'Counter-Strike: Source',
  },
  300: {
    gameId: 'dods',
    gameName: 'Day of Defeat: Source',
  },
  320: {
    gameId: 'hl2dm',
    gameName: 'Half-Life 2: Deathmatch',
  },
  440: {
    gameId: 'tf2',
    gameName: 'Team Fortress 2',
  },
  500: inf => ({
    gameId: 'l4d',
    gameName: 'Left 4 Dead',
    gameMode: inf.desc.substr(6),
  }),
  550: inf => {
    const modes: Record<string, string> = {
      coop: 'Co-op',
      realism: 'Realism',
      survival: 'Survival',
      versus: 'Versus',
      scavenge: 'Scavenge',
    }

    return {
      gameId: 'l4d2',
      gameName: 'Left 4 Dead ',
      gameMode: modes[inf.keywords || 'coop'],
    }
  },
  630: {
    gameId: 'as',
    gameName: 'Alien Swarm',
  },
  730: {
    gameId: 'csgo',
    gameName: 'Counter-Strike: Global Offensive',
  },
  17500: {
    gameId: 'zp',
    gameName: 'Zombie Panic!',
  },
  17520: {
    gameId: 'syn',
    gameName: 'Synergy',
  },
  17700: {
    gameId: 'ins',
    gameName: 'Insurgency',
  },
  17710: {
    gameId: 'nd',
    gameName: 'Nuclear Dawn',
  },
  41070: {
    gameId: 'ss3',
    gameName: 'Serious Sam 3',
  },
}

const sendPacket = (type: string, payload?: Uint8Array) => {
  const w = new DataWriter();

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

const normalizeServerInfo = (inf: ServerInfoExended) => {
  const ret: ServerInfo = {
    gameId: 'unknown',
    gameName: 'Unknown Game',
    name: inf.name,
    map: inf.map,
    numPlayers: inf.numPlayers,
    maxPlayers: inf.maxPlayers,
    version: inf.version,
    private: inf.private,
    secure: inf.secure,
  }

  const getMoreInfo = games[inf.appid || 0] || games[0]

  if (getMoreInfo) {
    const moreInfo = typeof getMoreInfo === 'function'
      ? getMoreInfo(inf)
      : getMoreInfo

    if (moreInfo) {
      Object.assign(ret, moreInfo)
    }
  }

  return ret
}

const readServerInfoGold = (r: DataReader) => {
  const ret: ServerInfoExended = {
    gameId: 'unknown',
    gameName: 'Unknown Game',
    address: r.zstring(),
    name: r.zstring(),
    map: r.zstring(),
    dir: r.zstring(),
    desc: r.zstring(),
    numPlayers: r.u8(),
    maxPlayers: r.u8(),
    numBots: 0,
    protocolVersion: r.u8(),
    serverType: r.lstring(1),
    osType: r.lstring(1),
    private: Boolean(r.u8()),
  }

  const mod = r.u8()

  if (ret.mod) {
    ret.mod = {
      link: r.zstring(),
      download: r.zstring(),
      null: r.u8(),
      version: r.i32le(),
      size: r.i32le(),
    }
  }

  ret.secure = Boolean(r.u8())
  ret.numBots = r.u8()

  return ret
}

const readServerInfo = (r: DataReader) => {
  const inf: ServerInfoExended = {
    gameId: 'unknown',
    gameName: 'Unknown Game',
    protocolVersion: r.u8(),
    name: r.zstring(),
    map: r.zstring(),
    dir: r.zstring(),
    desc: r.zstring(),
    appid: r.u16le(),
    numPlayers: r.u8(),
    maxPlayers: r.u8(),
    numBots: r.u8(),
    serverType: r.lstring(1),
    osType: r.lstring(1),
    private: Boolean(r.u8()),
    secure: Boolean(r.u8()),
  }

  if (inf.appid === 2400) { // The Ship
    inf.theShip = {
      mode: r.u8(),
      witnesses: r.u8(),
      duration: r.u8(),
    }
  }

  inf.version = r.zstring()
  const edf = r.u8()

  if (edf & 0x80) {
    inf.serverPort = r.u16le()
  }

  if (edf & 0x10) {
    gsw.print('warn: need to read \'inf.steamid\'')
    // inf.steamid = r.u64le()
    r.skip (8)
  }

  if (edf & 0x40) {
    inf.sourcetvPort = r.u16le()
    inf.sourcetvName = r.zstring()
  }

  if (edf & 0x20) {
    inf.keywords = r.zstring()
  }

  if (edf & 0x01) {
    gsw.print('warn: need to read \'inf.gameId\'')
    // inf.gameId = r.i64le()
    r.skip(8)
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
      gsw.sinfo(normalizeServerInfo(gotServerInfo))
      nextQuery ()
      break
    case 'm':
      gotServerInfo = readServerInfoGold(r)
      gotChallenge = -1
      gsw.sinfo(normalizeServerInfo(gotServerInfo))
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

const concatBuffers = (bufs: Uint8Array[]) => {
  const size = bufs.reduce((acc, buf) => acc + buf.byteLength, 0)
  let result = new Uint8Array(size)
  let offset = 0

  bufs.forEach(buf => {
    result.set(buf, offset)
    offset += buf.byteLength
  })

  return result
}

export const processResponse = (data: Uint8Array) => {
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
      const data = concatBuffers(req.packets)
      const r = new DataReader(data)
      readPayload(r)
      delete requests[pak.reqid]
    }
  }
}
