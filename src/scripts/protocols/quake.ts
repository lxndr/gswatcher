import { InvalidResponseError } from "../lib/response-error"

export const info: ProtocolInfo = {
  id: 'quake',
  name: 'Quake',
  feature: 'query',
  transport: 'udp',
}

export const parseQuakeInfo = (data: string): Record<string, string> => {
  if (!data.startsWith('\\')) {
    throw new InvalidResponseError('no leading backslash')
  }

  const parts = data.substr(1).split('\\')
  const count = parts.length

  const dict: Record<string, string> = {}

  for (let i = 0; i < count; i += 2) {
    const key = parts[i]
    const val = parts[i + 1]
    dict[key] = val
  }

  return dict
}
