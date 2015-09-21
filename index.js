var path = require('path')

var binary = require('node-pre-gyp')

module.exports = require(binary.find(
  path.resolve(path.join(__dirname, './package.json'))))
