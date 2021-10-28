const path = require('path')

module.exports = {
  entry: {
    source: './protocols/source.ts',
  },
  output: {
    filename: '[name].js',
    globalObject: 'globalThis',
    chunkFormat: 'module',
    library: {
      name: 'module',
      type: 'global',
    },
  },
  target: ['es5'],
  mode: 'production',
  optimization: {
    minimize: false,
  },
  resolve: {
    extensions: ['.ts'],
  },
  module: {
    rules: [
      { test: /\.ts$/, loader: 'ts-loader' },
    ],
  },
}
