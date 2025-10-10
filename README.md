monosc is a simple bash program made to simplify glslangValidator and SPIRV-Shadercross with a cmake-like command and file relationship. It supports GLSL, HLSL, and SPIRV as input, SPIRV, MSL, DXBC, and DXIL as output, and can compile vertex, fragment, and compute shaders.

Below is the syntax for the `monosc` command:
````
monosc <directory> [options]

parameters:
  <directory>         : The directory that houses your
                        shader_compile.json

options:
  --guess-hlsl-files  : Since HLSL does not have a strict naming
                        scheme, this option will guess what each
                        .hlsl file does. If you exclude this
                        option, you must prefix your file
                        extension with the shader type.

                        (.vert.hlsl, .frag.hlsl, or .comp.hlsl)

                        This option only works with "lazy": true
````

Below is the structure for your `shader_compile.json`:

**JSON structure (standard):**
````
.
├── lazy (false)
├── read_format [glsl, hlsl, spirv]
├── write_format [glsl, hlsl, spirv, msl, dxbc, dxil]
└── shaders
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
├── lazy (true)
├── read_format [glsl, hlsl, spirv]
├── write_format [glsl, hlsl, spirv, msl, dxbc, dxil]
└── shaders
    ├── read_folder "string/directory"
    ├── files (empty) ["string/filename"]
    └── write_folder "string/directory"
````
