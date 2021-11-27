import 'core-js/features/object/entries'
import { InvalidResponseError } from './lib/response-error'

export const info: ProtocolInfo = {
  id: 'gamespy',
  name: 'GameSpy',
  transport: 'udp',
}

const createPacket = (values: Record<string, string>) => {
  const str = Object.entries(values).map(([key, value]) => `\\${key}\\${value}`).join('')
  return new Buffer(str)
}

export const query = () => {
  const data = createPacket({
    basic: '',
    info: '',
    players: '',
  })

  gsw.send(data)
}

const parsePacket = (data: Buffer) => {
  const str = data.toString()

  if (str[0] !== '\\') {
    throw new InvalidResponseError('missing leading slash')
  }

  const parts = str.split('/')
  parts.shift()
  const num_pairs = parts.length / 2

  const inf: ServerInfo = {}

  for (let i = 0; i < num_pairs; i++) {
    const key = parts[i * 2]
    const val = parts[i * 2 + 1]
    gsw.print(`${key} = ${val}`)
  }

  return inf
}

export const processResponse = (data: Buffer) => {
  const inf = parsePacket(data)
  gsw.sinfo(inf)
}
