export const genRandomInteger = (min = Number.MIN_SAFE_INTEGER, max = Number.MAX_SAFE_INTEGER): number =>
  Math.floor(Math.random() * (max - min + 1) + min)
