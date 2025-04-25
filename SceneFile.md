# Scene File Format

This is a simple scene description format parsed word by word.

## General Notes

- Comments: `# this is a comment #` (# must be separated by whitespace, else it won't count as a comment special character)
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

All fields are optional unless otherwise noted.  
Textures are loaded from relative file paths.

```txt
new material <name> <type>
  <parameters>
end
```

### Phong

This material is based on the [Phong Reflection Model](https://en.wikipedia.org/wiki/Phong_reflection_model) with several additions such as transparency, emissive and simple subsurface scattering.

```txt
new material <name>
    diffuseColor <r> <g> <b> <a>
    diffuseTexture <path>
    specularColor <r> <g> <b> <a>
    specularTexture <path>
    tintColor <r> <g> <b> <a>
    tintTexture <path>
    emissiveColor <r> <g> <b> <a>
    emissiveTexture <path>
    normalMap <path> <POM>
    transparent
    doubleSided
end
```

#### Parameters

##### Colors

All colors can have a texture.  
If no texture is set, only the color is used.  
If a texture is set, texture samples are multiplied by the color, so make sure to set both.

- **Diffuse**: Aka albedo, aka base. If alpha < 0.5, fragments will not be drawn. (alpha cutout)
- **Specular**: Specular highlights. Alpha = 0 disables specular. Otherwise alpha is `10 * log2(shininess)`.
- **Tint**: Depends on whether the material is transparent:
  - **Transparent materials**: Filters light coming from behind the material (the existing pixels).  
    Alpha is ignored. Useful for tinted glass, for example.
  - **Non-transparent materials**: Controls subsurface scattering.  
    In other words, light hitting the back of a flat object creates diffuse lighting visible at the front.  
    Alpha controls how much the intensity depends on view direction. Most useful for leaves. Only works for flat materials.
- **Emissive**: This is added to the lighting calculation regardless of incoming light, as if the material emits this light itself.

##### Normal & displacement maps

Used to add detail that would otherwise require a lot of polygons. Red channel is X, green is Y, blue is Z, alpha is displacement.

POM parameter control Parallax [Occlusion] mapping:

- `-1`: Disable
- `0`: Parallax Mapping
- `>0`: Enable Parallax Occlusion Mapping, parameter controls steps. It is advised that the lowest amount that looks correct at high angles be used. A typical value is 5.

### Flags

- **Transparent**: Used for partially transparent materials like glass, but not needed for alpha cutout.  
  Causes object using this material to be rendered last so blending works.  
  Slightly worse for performance, and doesn't support intersecting polygons.
- **Double Sided**: Disables back-face culling for flat materials (like fences and foliage, or a sphere with alpha-cutout), and makes transparent materials hollow so the inside surface reflects light.

## Meshes

```txt
new mesh <name> obj <material-name> <path>
new mesh <name> sphere <stacks> <sectors> <material-name>
new mesh <name> plane <subdivision x> <subdivision y> <material-name>
```

- `obj`: loads a mesh from .obj file
- `sphere`: procedurally generates a sphere. More stacks and sectors makes the object smoother at the cost of performance.
- `plane`: procedurally generates a flat plane. Subdivision should be kept at 1 unless the object is large, in which case it should be increased.

## Objects

```txt
new object <mesh-name>
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
new material MyMaterial phong
    diffuseColor 1 0 0 1
end

# Mesh and object #
new mesh MyMesh sphere 16 32 MyMaterial
new object MyMesh
    0 0 0
    1 1 1
    30 45 60
```

## Notes

- Material and mesh names cannot contain spaces.
- Indentation is for readability only. All whitespace is ignored.
- If the file is missing optional sections, defaults are used:
  - All camera settings, settings and scene globals are optional.
  - Material colors are optional and default to black/none.
  - Material textures are optional and default to null pointer.
