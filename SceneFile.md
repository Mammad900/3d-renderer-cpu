# Scene File Format

This is a simple scene description format parsed word by word.

## General Notes

- Comments: `# this is a comment #`
- Words are separated by any whitespace desired.
- Rotations are in degrees.
- All colors are 4 floats (r g b a).
- All vectors are 3 floats (x y z).
- All floats are decimal unless otherwise noted.

## Camera Settings

```txt
cam pos <x> <y> <z>         # camera position
cam rot <x> <y> <z>         # camera rotation (in degrees)
nearFar <near> <far>        # near and far clipping planes
fov <float>                 # field of view in degrees
```

## Settings

```txt
set renderMode <int>             # 0 is normal, 1 is z buffer
set backFaceCulling <0|1>        # turn off for slightly better visuals
set reverseAllFaces <0|1>        # flip all face winding
set fullBright <0|1>             # disable all lighting
set wireFrame <0|1>              # show wire-frames
set whitePoint <float>           # HDR white point control
```

## Scene Globals

```txt
fog <r> <g> <b> <a>              # fog color (a defines intensity)
ambientLight <r> <g> <b> <a>     # ambient lighting color (a defines intensity)
```

## Lights

Alpha is light intensity.

```txt
new light directional <x> <y> <z>  <r> <g> <b> <a>   # x y z is rotation
new light point <x> <y> <z>  <r> <g> <b> <a>         # x y z is position
```

## Materials

All these fields are optional.

Textures are loaded from relative file paths.

```txt
new material
    diffuseColor <r> <g> <b> <a>   # Base color. If alpha = 0, diffuse is disabled, even the texture. Otherwise, alpha is unused.
    diffuseTexture <path>          # Texture that replaces diffuse color. If not null, diffuse color (except alpha) will be ignored. 
                                   # This texture's alpha channel is handled differently: if a pixel's alpha < 0.5, it will not be drawn. (alpha cutout)
    specularColor <r> <g> <b> <a>  # Specular highlights color. Alpha = 0 disables specular. Otherwise alpha = 10 * log2(shininess).
    specularTexture <path>         # Texture for specular. Behaves the same way and overrides the color field.
    tintColor <r> <g> <b> <a>      # [todo] Transparent materials only: filters light coming from behind the material. Useful for tinted glass, for example.
    tintTexture <path>
    emissiveColor <r> <g> <b> <a>  # Emissive: this color is added to the lighting calculation regardless of incoming light, as if the material emits this light itself.
    emissiveTexture <path>
    normalMap <path>
    transparent                    # [todo] flag: 
                                   # Causes the polygons to be rasterized last, and allows polygons behind them to be partially visible. 
                                   # Slightly worse for performance, and polygons can't intersect.
                                   # Not needed for alpha cutout.
    disableBackfaceCulling         # flag: Enable for transparent materials (including alpha cutout). This allows back-faces which would normally be culled to be visible.
end
```

## Meshes

```txt
new mesh obj <materialIndex> <path>
new mesh sphere <stacks> <sectors> <materialIndex>
```

- `obj`: loads a mesh from .obj file
- `sphere`: procedurally generates a sphere

---

## Objects

```txt
new object <meshIndex>
    <x> <y> <z>         # position
    <sx> <sy> <sz>      # scale
    <rx> <ry> <rz>      # rotation (degrees)
```

## Example

```txt
# Camera setup #
cam pos 0 0 5
cam rot 0 0 0
nearFar 0.1 100
fov 70

# Lighting #
ambientLight 0.2 0.2 0.2 1
new light directional 1 -1 -1 1 1 1 1

# Material #
new material
    diffuseColor 1 0 0 1
end

# Mesh and object #
new mesh sphere 16 32 0
new object 0
    0 0 0
    1 1 1
    0 0 0
```

## Notes

- Indentation is for readability only. All whitespace is ignored.
- If the file is missing optional sections, defaults are used:
  - All camera settings, settings and scene globals are optional.
  - Material colors are optional and default to black/none.
  - Material textures are optional and default to null pointer.
