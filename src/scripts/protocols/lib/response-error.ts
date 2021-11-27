export function InvalidResponseError(message: string) {
  // @ts-expect-error
  this.message = message
  // @ts-expect-error
  this.name = 'InvalidResponseError'
  // @ts-expect-error
  return this
}

InvalidResponseError.prototype = Error.prototype
