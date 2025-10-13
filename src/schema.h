#pragma once

inline const char * shader_compile_schema =
R"({
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "title": "Schema for shader_compile.json",
    "description": "The schema that your shader_compile.json should follow",
    "type": "object",
    "properties": {
        "lazy": { "type": "boolean", "description": "Determines if you are using the lazy schema or not" },
        "read_format": { "type": "string", "enum": ["glsl", "hlsl", "spirv"], "description": "Determines the input shader code format" },
        "write_format": { "type": "string", "enum": ["spirv", "msl", "dxbc", "dxil"], "description": "Determines the output shader code format" },
        "shaders": {
            "type": "object",
            "properties": {
                "frag": {
                    "type": "object",
                    "properties": {
                        "read_folder": { "type": "string", "description": "Determines what folder path to read from" },
                        "files": { "type": "array", "description": "Optionally determines exactly what files to use" },
                        "write_folder": { "type": "string", "description": "Determines what folder path to write to" }
                    },
                    "required": ["read_folder", "files", "write_folder"],
                    "description": "Determines what fragment shader files to use, and their location"
                },
                "vert": {
                    "type": "object",
                    "properties": {
                        "read_folder": { "type": "string", "description": "Determines what folder path to read from" },
                        "files": { "type": "array", "description": "Optionally determines exactly what files to use" },
                        "write_folder": { "type": "string", "description": "Determines what folder path to write to" }
                    },
                    "required": ["read_folder", "files", "write_folder"],
                    "description": "Determines what vertex shader files to use, and their location"
                },
                "comp": {
                    "type": "object",
                    "properties": {
                        "read_folder": { "type": "string", "description": "Determines what folder path to read from" },
                        "files": { "type": "array", "description": "Optionally determines exactly what files to use" },
                        "write_folder": { "type": "string", "description": "Determines what folder path to write to" }
                    },
                    "required": ["read_folder", "files", "write_folder"],
                    "description": "Determines what compute shader files to use, and their location"
                },
                "read_folder": { "type": "string", "description": "Determines what folder path to read from in lazy mode" },
                "files": { "type": "array", "description": "Optionally determines exactly what files to use" },
                "write_folder": { "type": "string", "description": "Determines what folder path to write to in lazy mode" }
            }
        }
    },
    "required": ["lazy", "read_format", "write_format", "shaders"],
    "if": {
        "properties": {
            "lazy": { "const": false }
        },
        "required": ["lazy"]
    },
    "then": {
        "properties": {
            "shaders": {
                "required": ["frag", "vert", "comp"],
                "not": {
                    "anyOf": [
                        { "required": ["read_folder"] },
                        { "required": ["files"] },
                        { "required": ["write_folder"] }
                    ]
                }
            }
        }
    },
    "else": {
        "properties": {
            "shaders": {
                "required": ["read_folder", "files", "write_folder"],
                "not": {
                    "anyOf": [
                        { "required": ["frag"] },
                        { "required": ["vert"] },
                        { "required": ["comp"] }
                    ]
                }
            }
        }
    }
})";
