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
set godRays <float>              # Enables god rays with the given sample size
```

## Scene Globals

```txt
fog <r> <g> <b> <a>              # fog color (a defines intensity)
ambientLight <r> <g> <b> <a>     # ambient lighting color (a defines intensity)
skyBox <texture>                 # Sky-box texture (equirectangular uv mapping)
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
new material <name> phong
    diffuse <texture>
    specular <texture>
    tint <texture>
    emissive <texture>
    normalMap <path> <strength> <POM>
    transparent
    doubleSided
    alphaCutout
end
```

#### Parameters

##### Colors

All colors can have a texture.  
If no texture is set, only the color is used.  
If a texture is set, texture samples are multiplied by the color, so make sure to set both.

- **Diffuse**: Aka albedo, aka base. If `alphaCutout` is enabled and alpha < 0.5, fragments will not be drawn and whatever is behind them will remain.
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

### Earth Material

```txt
new material <name> earth
    terrainDiffuse <texture>
    oceanDiffuse <texture>
    oceanSpecular <texture>
    oceanMask <texture>
    cityLights <texture>
    cloudDiffuse <texture>
    cloud <texture>
    normalMap <path> <strength> <POM>
end
```

### PBR Material

This material is based on Physically Based Rendering, which is more accurate than phong materials but slower. It uses the metallic-roughness workflow, Cook-Torrance BRDF, Fresnel-Schlick approximation, GGX normal distribution function, Smith geometry function with with Schlick-GGX approximation, energy conservation and Lambertian diffuse. (if you don't know what this means, don't worry, its good PBR)

```txt
new material <name> pbr
    albedo <color texture>
    metallic <float texture>
    roughness <float texture>
    ambientOcclusion <float texture>
end
```

#### Parameters

- **Albedo**: This is the base color of the material. Has the same role as Diffuse on phong material.
- **Metallic**: 0 for non-metallic, 1 for metallic. Metals have a much stronger base specular reflection and no diffuse. Values between 0 and 1 are only intended for texture filtering.
- **Roughness**: Defines how rough the surface is. The lower it is, the sharper reflections are.

### Flags

- **Transparent**: Used for partially transparent materials like glass, but not needed for alpha cutout.  
  Causes object using this material to be rendered last so blending works. When deferred rendering is active, it will be drawn in forward.  
  Worse for performance, and doesn't support intersecting polygons, so only use for materials that need it.
- **Double Sided**: Disables back-face culling for flat materials (like fences and foliage, or a sphere with alpha-cutout), and makes transparent materials hollow so the inside surface reflects light.
- **Alpha Cutout**: When enabled, any fragment with base color (diffuse color in case of phong) less than 0.5 will be rejected.  
  In other words, transparent pixels in the base color texture will make the material transparent in the respective positions.  
  This will reduce some of the performance savings of deferred rendering, so use only on materials that use it. It's faster than the transparent flag, however.

### Textures

```txt
... color <r> <g> <b> <a>           # Solid color #
... texture <r> <g> <b> <a> <path>  # Texture loaded from file, given color is multiplied with each texel #
```

If the texture file couldn't be loaded, a black/magenta texture will replace it.

## Meshes

```txt
new mesh <name> obj <material-name> <path>
new mesh <name> stl <material-name> <path>
new mesh <name> sphere <stacks> <sectors> <material-name>
new mesh <name> plane <subdivision x> <subdivision y> <material-name>
new mesh <name> custom|raw          # see custom meshes section below #
    v <index> <x> <y> <z> <u> <v> [<nx> <ny> <nz>] # Raw meshes include normals #
    m <material-name>
    f <v1> <v2> <v3>
    flat
end
new mesh <name> spheroid <type> <material-name> [<parameters>]
```

### Custom meshes

To define a vertex: use command `v`, followed by the index (starts with 0), position and UV.

To define a face, first switch to the material you want to assign to the face with `m`, then use `f` followed by the indices of its three vertices.

There can be a maximum of 65536 vertices. (this actually applies to all meshes, not just custom ones)

Use `flat` to disable smooth shading, which is enabled by default.

### Spheroid Types

Consult google or wikipedia if you don't know what these words mean.

None of these are UV mapped.

- `regularIcosahedron`
- `subdividedIcosahedron <subdivision steps>` (subdivides by 2 and projects to sphere given number of times)
- `regularDodecahedron`
- `pentakisDodecahedron`
- `truncatedIcosahedron`
- `ball <pentagons-material-name> <subdivision steps>` (a truncated icosahedron with alternate material assigned to pentagonal faces, then subdivided and projected to a sphere)

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

Object transforms propagate. This means that position, scale and rotation affect the children as well. For example, rotating an object causes its children to rotate around its center.

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
               1 <resolution x> <resolution y>  # Enable shadow mapping #
               0                                # Or disable #
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
        whitePoint <float>    # HDR tone-mapping white point, default is the brightest pixel #
    end
...
```

#### Rotator

Applies a constant rotation to its object.

```txt
...
    rotator <rx> <ry> <rz> # Speed in degrees per second  #
...
```

#### Keyboard Control

Allows the object's rotation to be controlled with arrow keys, and scale with num -+.

```txt
...
    keyboardControl <sx> <sy> <sz> # Speed, x and y for look and z for zoom #
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
