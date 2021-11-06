export const info: ProtocolInfo = {
  id: 'gamespy',
  name: 'GameSpy',
  transport: 'udp',
}

export const query = () => {
  const query = '\\basic\\\\info\\\\players\\'
  gsw.send(Buffer.from(query))
}

export const processResponse = (data: Buffer) => {
  const resp = data.toString()
  gsw.print(resp)
}
