const path = require('path')

module.exports = {
  entry: {
    gamespy: './protocols/gamespy.ts',
    quake3: './protocols/quake3.ts',
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
    pathinfo: true,
  },
  mode: 'production',
  optimization: {
    minimize: false,
  },
  resolve: {
    extensions: ['.js', '.ts', '.mjs'],
    alias: {
      jsbi: path.resolve(__dirname, 'node_modules/jsbi/dist/jsbi.mjs'),
    },
  },
  module: {
    rules: [
      {
        test: /\.(ts|js|mjs)$/,
        use: {
          loader: 'babel-loader',
        },
      },
    ],
  },
}
