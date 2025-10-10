Below is the structure for your `shader_compile.json`:

**JSON structure (standard):**
``
.
└── shaders
    ├── lazy (false)
    ├── read_format [glsl, hlsl, spirv, ]
    ├── write_format [glsl, hlsl, spirv, ]
    ├── frag
    │   ├── read_folder
    │   ├── files (empty)
    │   └── read_folder
    ├── vert
    │   ├── read_folder
    │   ├── files (empty)
    │   └── read_folder
    └── comp
        ├── read_folder
        ├── files (empty)
        └── read_folder
``

**JSON structure (lazy):**
``
.
└── shaders
    ├── lazy [true]
    ├── read_folder
    └── write_folder
``
