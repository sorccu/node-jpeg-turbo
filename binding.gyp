{
  "targets": [
    {
      "target_name": "JpegTurbo",
      "sources": ["src/binding.cc"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "libraries": [
        "-lturbojpeg"
      ],
      "conditions": [
        ["OS=='mac'", {
          "include_dirs": [
            "/usr/local/opt/jpeg-turbo/include"
          ],
          "link_settings": {
            "library_dirs": [
              "/usr/local/opt/jpeg-turbo/lib"
            ]
          }
        }]
      ]
    }
  ]
}
