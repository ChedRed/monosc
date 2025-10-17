monosc is a simple bash program made to simplify glslangValidator and SPIRV-Shadercross with a cmake-like command and file relationship. It supports GLSL, HLSL, and SPIRV as input, SPIRV, MSL, DXBC, and DXIL as output, and can compile vertex, fragment, and compute shaders.

Notes:
* .hlsl files must be suffixed with .frag, .vert, .comp, etc. (.frag.hlsl ...)
* Checks for files (when "files" is unspecified) in the specified directory are not recursive.

Current caveats:
* .spv files must also be suffixed with .frag, .vert, .comp, etc. (.frag.spv ...)
* DXBC and DXIL cannot be compiled from macos until I get mach-dxcompiler compiled on my machine.


Below is the syntax for the `monosc` command:
````
monosc <directory>

parameters:
  <directory>         : The directory that houses your
                        shader_compile.json
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

**Multistage**
GLSL -> DXBC, DXIL: glslangValidator -> spirv-shadercross (HLSL) -> dxc
GLSL -> HLSL, MSL: glslangValidator -> spirv-shadercross
HLSL -> GLSL, MSL: glslangValidator -> spirv-shadercross
