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
  [InfoField.MAP]: String(inf.maptitle),
  [InfoField.GAME_VERSION]: String(inf.gamever),
})

export const processResponse = (data: Buffer) => {
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

    const inf = normalizeServerInfo(all_info)
    gsw.details(all_info)
    gsw.sinfo(inf)
  }
}
