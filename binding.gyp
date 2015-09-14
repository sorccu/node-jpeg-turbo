{
  "targets": [
    {
      "target_name": "JpegTurbo",
      "sources": ["src/binding.cc"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "/usr/local/opt/jpeg-turbo/include"
      ],
      "link_settings": {
        "libraries": ["-L/usr/local/opt/jpeg-turbo/lib", "-lturbojpeg"]
      }
    }
  ]
}
