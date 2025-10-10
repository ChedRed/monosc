Below is the structure for your `shader_compile.json`:

**JSON structure (standard):**
````
.
└── shaders
    ├── lazy (false)
    ├── read_format [glsl, hlsl, spirv]
    ├── write_format [glsl, hlsl, spirv, msl, dxbc, dxil]
    ├── frag
    │   ├── read_folder "string/directory"
    │   ├── files (empty) ["string/filename"]
    │   └── read_folder "string/directory"
    ├── vert
    │   ├── read_folder "string/directory"
    │   ├── files (empty) ["string/filename"]
    │   └── read_folder "string/directory"
    └── comp
        ├── read_folder "string/directory"
        ├── files (empty) ["string/filename"]
        └── read_folder "string/directory"
````

**JSON structure (lazy):**
````
.
└── shaders
    ├── lazy (true)
    ├── read_format [glsl, hlsl, spirv]
    ├── write_format [glsl, hlsl, spirv, msl, dxbc, dxil]
    ├── read_folder "string/directory"
    ├── files (empty) ["string/filename"]
    └── write_folder "string/directory"
````
