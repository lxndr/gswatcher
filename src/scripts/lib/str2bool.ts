const true_strings = [
  '1',
  'true',
  'on',
  'yes',
]

export const str2bool = (str: string) =>
  true_strings.indexOf(str.toLowerCase()) !== -1
