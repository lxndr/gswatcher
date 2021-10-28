export {}

declare global {
  interface ProtocolInfo {
    name: string
    version: string
    transport: string
  }

  interface ServerInfo {
    gameId: string
    gameName: string
    name: string
    map?: string
    version?: string
    numPlayers?: number
    maxPlayers?: number
    serverType?: string
    osType?: string
    private?: boolean
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
    send(data: Uint8Array): void
    print(str: string): void
  }

  const gsw: Gsw
}
