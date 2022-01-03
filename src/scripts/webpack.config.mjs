import { dirname, resolve } from 'path'
import { fileURLToPath } from 'url'

const __dirname = dirname(fileURLToPath(import.meta.url));

export default {
  entry: {
    ase: './protocols/ase.ts',
    gamespy: './protocols/gamespy.ts',
    gamespy2: './protocols/gamespy2.ts',
    quake3: './protocols/quake3.ts',
    source: './protocols/source.ts',
    'source-console': './protocols/source-console.ts',
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
  target: 'es5',
  mode: 'production',
  optimization: {
    minimize: false,
  },
  resolve: {
    extensions: ['.ts', '.js'],
    mainFields: ['module', 'main'],
    alias: {
      jsbi: resolve(__dirname, 'node_modules/jsbi/dist/jsbi-cjs.js'),
    }
  },
  module: {
    rules: [
      {
        test: /\.(ts|js)$/,
        use: {
          loader: 'ts-loader',
        },
      },
    ],
  },
}
