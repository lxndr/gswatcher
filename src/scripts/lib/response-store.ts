import { InvalidResponseError } from './response-error'

export interface Packet<DATA> {
  reqid: number
  number: number
  data: DATA
}

export abstract class CompoundResponse<DATA, PACKET extends Packet<DATA>> {
  protected packets: PACKET[] = []
  public total = 0
  private firstPacketTime = new Date()

  abstract combine(): DATA

  addPacket(packet: PACKET) {
    const { number } = packet

    if (!(Number.isInteger(number) && number >= 0)) {
      throw new InvalidResponseError('packet number is not a positive integer')
    }

    if (this.total && number >= this.total) {
      throw new InvalidResponseError('incorrect packet')
    }

    this.packets[packet.number] = packet
  }

  gotAllPackets() {
    if (!this.total) {
      return false
    }

    for (let i = 0; i < this.total; i++) {
      if (!this.packets[i]) {
        return false
      }
    }

    return true
  }
}

export class ResponseStore<DATA, PACKET extends Packet<DATA>, RESPONSE extends CompoundResponse<DATA, PACKET>> {
  private responses: Record<number, RESPONSE> = {}

  constructor(private Response: new() => RESPONSE) {}

  addPacket(packet: PACKET) {
    const res = this.ensureResponse(packet.reqid)
    res.addPacket(packet)
    return res
  }

  ensureResponse(reqid: number) {
    if (!(Number.isInteger(reqid) && reqid >= 0)) {
      throw new InvalidResponseError('request id is not a positive integer')
    }

    if (!this.responses[reqid]) {
      this.responses[reqid] = new this.Response()
    }

    return this.responses[reqid]
  }

  remove(reqid: number) {
    delete this.responses[reqid]
  }
}
