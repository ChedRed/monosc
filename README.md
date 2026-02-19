monosc is a simple c++ program made to simplify glslangValidator and, SPIRV-Shadercross with a cmake-like command and file relationship. It supports GLSL, HLSL, and SPIRV as input, SPIRV, MSL, DXBC, and DXIL as output, and can compile vertex, fragment, and compute shaders. It supports adding entrypoints per file.

Notes:
* .hlsl files must be suffixed with .frag, .vert, .comp, etc. (.frag.hlsl ...)
* Checks for files (when "files" is unspecified) in the specified directory are not recursive.

Current caveats:
* .spv files must also be suffixed with .frag, .vert, .comp, etc. (.frag.spv ...)
* DXBC and DXIL cannot be compiled until I get mach-dxcompiler and other dx-compilers compiled on my machine.


Below is the syntax for the `monosc` command:
````
monosc <path>

parameters:
  <path>            : The path that houses your
                        shader_compile.json, or
                        a path to a .json file.
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


Here's what the shader files are processed through for each type:


**Direct**

GLSL, HLSL -> SPIRV: glslangValidator

SPIRV -> GLSL, HLSL, MSL: spirv-shadercross

HLSL -> DXBC, DXIL: dxc

HLSL -> METALLIB: metal cli

**Multi-stage**

GLSL -> DXBC, DXIL: glslangValidator -> spirv-shadercross (HLSL) -> dxc

GLSL/HLSL -> GLSL, HLSL, MSL: glslangValidator -> spirv-shadercross

SPIRV -> METALLIB: spirv-shadercross (MSL) -> metal cli
