# Scene File Format

This is a simple scene description format parsed word by word.

## General Notes

- Comments: `# this is a comment #` (# must be separated by whitespace, else it won't count as a comment special character)
- Words are separated by any whitespace desired.
- Rotations are in degrees.
- All colors are 4 floats (r g b a).
- All vectors are 3 floats (x y z).
- All floats are decimal unless otherwise noted.

## Files

To run another scene file:

```txt
import path/to/file.txt
```

The opened file will inherit the currently editing scene of the file opening it (caller), but switching to another scene within that file does not affect the caller.

## Scenes

All data below is part of a scene. You must have at least one scene at a time, and switch to a scene before running any other command.

```txt
scene new MyScene           # Create a new scene with default settings
scene edit MyScene          # Switch to editing the scene, all following commands in this file will edit this scene
scene render MyScene        # Set the scene as the one being rendered
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
new object <name>
    <x> <y> <z>         # position
    <sx> <sy> <sz>      # scale
    <rx> <ry> <rz>      # rotation (degrees)
    <components and children>
end
```

Each object contains one or more components, or one or more children, or both, or neither.

Object transforms propagate. This means that position, scale and rotation affect the children as well. For example, rotating an object causes its children to rotate around its center.s

### Possible components

#### Mesh

```txt
...
    mesh <mesh name>
...
```

#### Light

Alpha is light intensity.

```txt
...
    light directional <r> <g> <b> <a>
    light point <r> <g> <b> <a>
    light spot <r> <g> <b> <a> <spread inner> <spread outer> # Spread angle in degrees #
...
```

- Directional lights:
  - Fastest
  - Same intensity and direction everywhere
  - Example: The sun
  - Use object rotation to adjust light direction. At 0 rotation, the light shines at +Z.
- Point lights:
  - Medium speed
  - Intensity is proportional to inverse square of distance to light, direction is always from light
  - Example: A light bulb
  - Uses object position
- Spot light:
  - Like a point light but limited to a cone
  - Slower than point lights per area affected, but lower affected area can make it much faster than point lights + shadows
  - Intensity and direction behaves like point lights
  - Example: Street lights, car headlights
  - Use object rotation and position to adjust light cone. At 0 rotation, the light shines at +Z.

#### Camera

There must be at least one camera per scene. The last camera created is used to render the scene. At 0 rotation, the camera looks at +Z.

```txt
...
    camera
        fov <fov>             # in degrees, default is 60 #
        nearFar <near> <far>  # Default is 0.1 and 100 #
    end
...
```

## Example

```txt
# Camera setup #
new object
    0 0 5
    1 1 1
    0 0 0
    camera
      fov 70
    end
end

# Lighting #
ambientLight 0.2 0.2 0.2 1
new object
    0 0 0
    1 1 1
    0 90 0
    light directional 1 1 1 2
end

# Material #
new material MyMaterial phong
    diffuseColor 1 0 0 1
end

# Mesh and object #
new mesh MyMesh sphere 16 32 MyMaterial
new object
    0 0 0
    1 1 1
    30 45 60
    mesh MyMesh
end
```

## Notes

- Material and mesh names cannot contain spaces.
- Indentation is for readability only. All whitespace is ignored.
- If the file is missing optional sections, defaults are used:
  - All camera settings, settings and scene globals are optional.
  - Material textures/colors are optional and default to black/none.
