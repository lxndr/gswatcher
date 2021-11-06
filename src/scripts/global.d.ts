export {}

declare global {
  interface ProtocolInfo {
    id: string
    name: string
    transport: string
  }

  interface ServerInfo {
    name: string
    game_mode?: string
    map?: string
    version?: string
    num_players?: number
    max_players?: number
    server_type?: string
    os?: string
    has_password?: boolean
    secure?: boolean
  }

  interface Player {
    index: number
    name: string
    score?: number
    duration?: number
    deaths?: number
    money?: number
  }

  class Gsw {
    sinfo(inf: ServerInfo): void
    plist(pl: Player[]): void
    send(data: Buffer): void
    print(str: string): void
  }

  const gsw: Gsw
}
