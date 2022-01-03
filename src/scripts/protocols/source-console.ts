import '../lib/polyfill'
import { AuthError } from '../lib/auth-error'
import { DataReader } from '../lib/data-reader'
import { DataWriter } from '../lib/data-writer'
import { InvalidResponseError } from '../lib/response-error'

export const info: ProtocolInfo = {
  id: 'source-console',
  name: 'Source Engine Remote Console',
  feature: 'console',
  transport: 'tcp',
  options: [
    { id: 'password', type: 'password' },
  ],
}

const enum PacketType {
  AUTH = 3,
  AUTH_RESPONSE = 2,
  EXEC_COMMAND = 2,
  RESPONSE_VALUE = 0,
}

let pending_command = ''
let authorized = false
let requestId = 0
let buffer = Buffer.alloc(0)

const createPacket = (id: number, type: PacketType, body = '') => {
  const length = body.length + 10
  const w = new DataWriter ()
    .i32le(length)
    .i32le(id)
    .i32le(type)
    .zstring(body)
    .zstring('')
  return w.buf
}

export const sendCommand = (cmd: string, options?: Record<string, string>) => {
  if (!authorized) {
    if (options?.password) {
      gsw.send(createPacket(requestId, PacketType.AUTH, options.password))
    }
  } else {
    gsw.send(createPacket(requestId, PacketType.EXEC_COMMAND, cmd))
  }
}

const parsePacket = (buf: Buffer) => {
  const r = new DataReader(buf)

  return {
    length: r.i32le(),
    id: r.i32le(),
    type: r.i32le(),
    body: r.zstring(),
    empty: r.zstring(),
  }
}

export const processResponse: ProcessResponseFn = data => {
  buffer = Buffer.concat([buffer, data])

  const length = buffer.readInt32LE(0)

  if (length >= (buffer.length - 4)) {
    return
  }

  const pak = parsePacket(buffer.slice(0, length))

  switch (pak.type) {
    case PacketType.AUTH_RESPONSE:
      if (pak.id === requestId) {
        authorized = true

        if (pending_command) {
          sendCommand(pending_command)
        }
      } else if (pak.id === -1) {
        throw new AuthError()
      } else {
        throw new InvalidResponseError()
      }
      break
    case PacketType.RESPONSE_VALUE:
      if (pak.id === requestId) {
        gsw.response(pak.body)
      } else {
        throw new InvalidResponseError()
      }
      break
    default:
      throw new InvalidResponseError()
  }

  buffer = buffer.slice(length + 4)
}
