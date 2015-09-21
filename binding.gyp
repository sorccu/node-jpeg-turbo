{
  'targets': [
    {
      'target_name': 'exports',
      'sources': [
        'src/decompress.cc',
        'src/exports.cc',
      ],
      'include_dirs': [
        '<!(node -e \'require("nan")\')'
      ],
      'dependencies': [
        'deps/libjpeg-turbo.gyp:jpeg-turbo'
      ]
    }
  ]
}
