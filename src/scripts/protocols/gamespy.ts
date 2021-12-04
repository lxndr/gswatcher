import 'core-js/features/object/entries'
import { CompoundResponse, Packet, ResponseStore } from './lib/response-store'
import { InvalidResponseError } from './lib/response-error'
import { parseQuakeInfo } from './quake'

export const info: ProtocolInfo = {
  id: 'gamespy',
  name: 'GameSpy',
  transport: 'udp',
}

interface GamespyPacket extends Packet<Info> {
  final: boolean
}

class GamespyResponse extends CompoundResponse<Info, GamespyPacket> {
  combine() {
    const infos = this.packets.map(packet => packet.data)
    return Object.assign({}, ...infos)
  }
}

class GamespyResponseStore extends ResponseStore<Info, GamespyPacket, GamespyResponse> {
  constructor() {
    super(GamespyResponse)
  }

  addPacket(packet: GamespyPacket) {
    const { reqid, number: fragment, final } = packet
    const res = this.ensureResponse(reqid)

    if (final) {
      if (res.total) {
        throw new InvalidResponseError('multiple final packets')
      }
  
      res.total = fragment + 1
    }

    return super.addPacket(packet)
  }
}

const responses = new GamespyResponseStore()

const createPacket = (values: Record<string, string>) => {
  const str = Object.entries(values).map(([key, value]) => `\\${key}\\${value}`).join('')
  return new Buffer(str)
}

export const query = () =>
  gsw.send(createPacket({
    basic: '',
    info: '',
    players: '',
  }))

const normalizeServerInfo = (inf: Info): ServerInfo => ({
  [InfoField.SERVER_NAME]: String(inf.hostname),
  [InfoField.NUM_PLAYERS]: Number(inf.numplayers),
  [InfoField.MAX_PLAYERS]: Number(inf.maxplayers),
  [InfoField.MAP]: String(inf.maptitle || inf.mapname),
  [InfoField.GAME_VERSION]: String(inf.gamever),
  [InfoField.GAME_MODE]: String(inf.gametype || inf.gamemode),
  [InfoField.PRIVATE]: inf.password === '1',
})

const normalizePlayerList = (inf: Info) => {
  const all_keys = Object.keys(inf)
  const players: Player[] = []

  for (let i = 1; ; i++) {
    const ending = `_${i}`
    const keys = all_keys.filter(key => key.endsWith(ending))

    if (!keys.length) {
      break;
    }

    const player: Player = {}

    keys.forEach(key => {
      const fleid = key.slice(0, -ending.length)
      player[fleid] = inf[key]
    })

    players.push(player)
  }

  return players
}

export const processResponse: ProcessResponseFn = data => {
  const { queryid, final, ...info } = parseQuakeInfo(data.toString())

  if (!queryid) {
    throw new InvalidResponseError('no queryid found')
  }

  const [reqid_str, fragment_str] = queryid.split('.')
  const reqid = parseInt(reqid_str, 10)
  const fragment = parseInt(fragment_str, 10)

  const res = responses.addPacket({
    reqid,
    number: fragment - 1,
    final: final != null,
    data: info,
  })

  if (res.gotAllPackets()) {
    const all_info = res.combine()
    responses.remove(reqid)
    gsw.details(all_info)

    const inf = normalizeServerInfo(all_info)
    gsw.sinfo(inf)

    const players = normalizePlayerList(all_info)
    gsw.plist(players)
  }
}
