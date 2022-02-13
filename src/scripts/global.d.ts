export {}

declare global {
  interface ProtocolOption {
    type: 'password'
    label: string
  }

  interface ProtocolInfo {
    id: string
    name: string
    feature: 'query' | 'console'
    transport: 'udp' | 'tcp'
    options?: Record<string, ProtocolOption>
  }

  interface Info {
    [key: string]: unknown
  }

  interface PlayerField {
    title: string
    field: string
    kind: 'string' | 'number' | 'duration'
    main: boolean
  }

  interface Player {
    [key: string]: unknown
  }

  const enum InfoField {
    SERVER_NAME = 'server-name',
    SERVER_TYPE = 'server-type',
    SERVER_OS = 'server-os',
    GAME_ID = 'game-id',
    GAME_NAME = 'game-name',
    GAME_MODE = 'game-mode',
    GAME_VERSION = 'game-version',
    MAP = 'map',
    NUM_PLAYERS = 'num-players',
    MAX_PLAYERS = 'max-players',
    PRIVATE = 'private',
    SECURE = 'secure',
  }

  interface ServerInfo {
    [InfoField.GAME_ID]?: string
    [InfoField.GAME_NAME]?: string
    [InfoField.GAME_MODE]?: string
    [InfoField.GAME_VERSION]?: string
    [InfoField.SERVER_NAME]?: string
    [InfoField.SERVER_TYPE]?: string
    [InfoField.SERVER_OS]?: string
    [InfoField.MAP]?: string
    [InfoField.NUM_PLAYERS]?: number
    [InfoField.MAX_PLAYERS]?: number
    [InfoField.PRIVATE]?: boolean
    [InfoField.SECURE]?: boolean
  }

  type QueryFn = () => void
  type ProcessResponseFn = (data: Buffer) => void
  type SendCommandFn = (cmd: string) => void

  class Gsw {
    sinfo(details: Info, inf: ServerInfo): void
    plist(pfields: PlayerField[], pl: Player[]): void
    send(data: Buffer): void
    print(str: string): void

    response(body: string): void
  }

  const gsw: Gsw
}
