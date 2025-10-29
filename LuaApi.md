## Simple types

### `Vec3`

A three dimensional single precision float vector.

```lua
my_vector = Vec3.new() -- Vec3.new(0, 0, 0)
my_vector = Vec3.new(1, 2, 3)
my_vector = Vec3.new{x= 1, z= 3} -- y defaults to 0

print(my_vector.x) -- 3
```

> [!NOTE]
> Most places that accept Vec3 can also accept the following shorthands. One notable exception is assignment.

```lua
something(Vec3.new(1, 2, 3))
something{1, 2, 3}
something(1) -- == something{1, 1, 1}
something(nil) -- == something{0, 0, 0}
```

#### `deg_to_rad` and `rad_to_deg`

```lua
my_vector = deg_to_rad{90, 45, 15}
my_vector = rad_to_deg{3.14, 0, 0}
```

### `Color`

An RGBA single precision float color. (0-1 range)

```lua
my_color = Color.new() -- Color.new(1,1,1,1)
my_color = Color.new(0.1, 0.2, 0.3, 1)
my_color = Color.new{r= 1} -- r,g,b default to 0, a defaults to 1
```

> [!NOTE]
> Most places that accept Color can also accept the following shorthands. One notable exception is assignment.

```lua
something(Color.new(0.1, 0.2, 0.3, 1))
something{0.1, 0.2, 0.3} -- a defaults to 1
something(0.1) -- == something{0.1, 0.1, 0.1, 1}
something(nil) -- == something{1, 1, 1, 1}
```

## `Scene`

Self explanatory name. Scenes contain objects. A scene must have an active camera.

```lua
my_scene = Scene.new() -- No parameters
set_render_scene(my_scene)
```

Fields and methods:

- **`name`** (string): Used in the GUI
- **`set_active_camera(camera)`**: Sets a `Camera` as the active one. Alternatively set `active` to true in the camera.
- **`objects`** (vector\<Object>): List of top level objects in the scene. Read only.
- **`add_object(object)`**: Add an object to the scene.
- **`add_objects{object1, object2, ...}`**: Add multiple objects to the scene.
- **`sky_box`**: An equirectangular texture rendered at infinite distance.
- **`back_face_culling`** (boolean): Defaults to true. When set to false, back-face culling is disabled. Can decrease performance especially in forward mode. Useful for debugging face winding.
- **`ambient_light`** (Color): Ambient lighting. Contributes to the lighting of every pixel (as long as the material allows it)
- **`volume`** (Volume): The global volume. If nil (default), a volume with 100% transmission is assumed.
- **`god_rays`** (boolean): Defaults to false. Enables ray-marched god-rays, aka volumetric lighting.
- **`god_rays_sample_size`** (number): Sample size used to ray-march god-rays. The lower, the slower, but higher quality. Default is 1.
- **`bilinear_shadow_filtering`** (boolean): Defaults to true. Enables bilinear filtering for shadow maps. Otherwise nearest neighbor is used.
- **`shadow_bias`** (number): Controls the shadow bias. Higher values decrease accuracy, but too low values may cause a visual glitch called shadow acne, which gets worse as the surface angle increases.
- **`texture_filtering_mode`** (enum): Controls the filtering method used when sampling textures.
  - `nearest_neighbor`: Textures appear blocky. Fastest but ugliest.
  - `bilinear`: Textures appear smooth, as the color gets interpolated between texels.
  - `trilinear`: Same as bilinear but mipmaps are interpolated. Can make textures blurry.

## `Object`

```lua
Object.new{
    name= "...",
    position= {0, 0, 0}, 
    rotation= deg_to_rad{0, 0, 0},
    scale= {1, 1, 1},
    children= {
        ...
    },
    components= {
        ...
    }
}
```

Fields and methods:

- **`name`** (string): Used in the GUI.
- **`position`** (Vec3): Object's position relative to its parent.
- **`rotation`** (Vec3): Object's rotation relative to its parent, in radians.
- **`scale`** (Vec3): Object's scale. Only use for objects which only contain meshes.
- **`children`** (vector\<Object>): List of child objects. Can be passed as lua array to the constructor.
- **`components`** (vector\<Component>): List of components. Components add behavior to the object. Can be passed as lua array to the constructor.
- **`add_child(object)`**: Adds an object to the children.
- **`add_component(component)`**: Adds a component to the object.

## `Component`

Components add behavior to objects, like mesh instances, light sources or cameras. It has multiple types, shown below.

> [!Warning]
> Do not pass components directly to objects. You have to use the method `as_component()` to convert it to the base type.

### `MeshComponent`

Instances a Mesh to be rendered at the object's transform.

```lua
my_component = MeshComponent(my_mesh):as_component()
```

### `Camera`

Renders what is in front of it to the screen.

```lua
my_camera = Camera.new{
    fov = 60,
    near = 0.1,
    ...
}:as_component()
```

- **`fov`**: Camera field of view, in degrees.
- **`near`**: Near clip distance. Fragments closer than this won't render.
- **`far`**: Far clip distance, aka render distance. Fragments farther than this won't render. When god-rays are enabled, also controls ray-marching distance for sky-box fragments. Lower values improve performance.
- **`white_point`**: Affects tone-mapping. Higher values make the render result dimmer and avoid overexposure. If set to 0, the brightest pixel is used to control white point.

### `RotatorComponent`

Adds a constant rotation to the object.

```lua
my_rotator = RotatorComponent.new(deg_to_rad{90, 0, 0})
...
my_rotator.enabled = false
...
my_rotator.enabled = true
my_rotator.rotate_per_second = deg_to_rad{45, 0, 0}
```

### `Light`

They emit light. There are several types of them. Each light has a color, with the alpha determining the intensity.

#### `DirectionalLight`

The simplest one. Shines a fixed amount of light everywhere in a constant direction, as if it were infinitely far away. Useful as the Sun. Object rotation affects light direction.

```lua
my_light = DirectionalLight.new(Color.new(1,1,1,10)):as_component()
```

#### `PointLight`

Shines light from the object position to everywhere, with intensity proportional to the inverse square of the distance.

```lua
my_light = PointLight.new(Color.new(1,1,1,10)):as_component()
```

#### `SpotLight`

Like `PointLight` but it only shines at one direction with a fixed angle. Object rotation affects light direction.

```lua
my_light = SpotLight.new(Color.new(1,1,1,10), 0.1, 0.2):as_component()
```

Parameters:

1. Color
2. Inner spread, in radians.
3. Outer spread, in radians.

## `Mesh`

A mesh consists of vertices and faces.

### `Vertex`

A vertex is a point to which faces attach. It also defines its normal and UV, which get interpolated between vertices.

```lua
Vertex.new{
    position= {1, 2, 3},
    uv= {0.2, 0.1},
    normal= {1, 0, 0}
}
```

> [!Note]
> 
> 1. `Vertex.new` can be omitted while using the Mesh constructor, and the table can be directly passed.
> 2. Apparently all normals in this engine are reversed.
> 3. Normals are optional (and overridden) if you enable `auto_normals` or `flat_shading`. See `Mesh.new` below.


### `Face`

A face, aka a triangle, is the graphics primitive, a surface that intersects three vertices. A face has a direction, which depends on its winding, affecting back-face culling. To be honest, I don't know which direction/winding this engine uses.

```lua
-- Any of these can be put in the faces array passed to Mesh.new
Face.new{v1= 0, v2= 1, v3= 2, material= my_material}
Face.new(0, 1, 2, my_material)
{0, 1, 2, my_material}
```

> [!Note]
> Notice how the vertices are set to numbers. To reduce memory usage and allow vertices to be shared between faces, faces only store the indices of the vertices. Indices start with 0, unlike in Lua arrays.

### `Mesh.new`

You can create meshes by manually defining their vertices and faces. There are also mesh generators and loaders available, shown below this section.

```lua
my_mesh = Mesh.new{
    label= "Just a mesh",
    flat_shading= false,
    auto_normals= true,
    vertices= {
        ...
    },
    faces= {
        ...
    }
}
```

- **`label`**: Used in the GUI
- **`flat_shading`**: If false, surface normals are interpolated between vertex normals, giving the mesh a smooth look. If true, vertex normals are ignored and a constant normal is used across each face, giving the mesh a flat and blocky look.
- **`auto_normals`**: If true, normals defined in the vertices are ignored, and set to the average of the normals of faces connected to each vertex.
- **`vertices`**: An array of vertices, see above.
- **`faces`**: An array of faces, see above.

### `generate_mesh`

```lua
my_mesh = generate_mesh{
    type= "see_below",
    name= "A mesh"
    ...
}
```

#### `obj`

Loads an OBJ file. At the moment UV and normals aren't supported (normals are always autogenerated), and the faces must be triangulated.

- **`material`**: Material to assign to every face. Required.
- **`file`**: Path to the OBJ file relative to the calling script.

#### `stl`

Loads an STL file. Both ASCII and binary STL files are supported. API is the same as `obj`, so refer to above.

#### `sphere`

A UV sphere made of stacks and sectors.

- **`material`**: Material to assign to every face. Required.
- **`stacks`**: Rows, affects face count vertically. Default 20.
- **`sectors`**: Columns, affects face count horizontally. Recommended to be double the stacks. Default 40.
- **`invert_u`**: If true, flips textures horizontally.
- **`invert_v`**: If true, flips textures vertically.

#### `plane`

A flat plane / quad. Usually you don't need subdivisions unless the plane is very large, in which case subdivisions can help with frustum culling.

- **`material`**: Material to assign to every face. Required.
- **`subdivisions_x`**: Number of face columns. Default 1.
- **`subdivisions_y`**: Number of face rows. Default 1.

#### `regular_icosahedron`

A [regular icosahedron](https://en.wikipedia.org/wiki/Regular_icosahedron). Cannot be UV mapped.

- **`material`**: Material to assign to every face. Required.

#### `ico_sphere`

A regular icosahedron, subdivided and projected to a sphere several times, resulting in a [geodesic polyhedron](https://en.wikipedia.org/wiki/Geodesic_polyhedron). Cannot be UV mapped.

- **`material`**: Material to assign to every face. Required.
- **`subdivisions`**: Number of subdivision and projection steps. Each step quadruples the face count. Default 3.

#### `dodecahedron`

A [regular](https://en.wikipedia.org/wiki/Regular_dodecahedron) or [pentakis](https://en.wikipedia.org/wiki/Pentakis_dodecahedron) dodecahedron.

- **`material`**: Material to assign to every face. Required.
- **`is_pentakis`**: If true, a pentakis dodecahedron. If false, a regular dodecahedron.

#### `truncated_icosahedron`

A [truncated icosahedron](https://en.wikipedia.org/wiki/Truncated_icosahedron).

- **`material`**: Material to assign to every face. Required.

#### `ball`

A [truncated icosahedron](https://en.wikipedia.org/wiki/Truncated_icosahedron), subdivided and projected to a sphere several times, resulting in a mesh that looks like an association football ball.

- **`material`**: Material to assign to every face. Required if the other two materials aren't set.
- **`hexagons_material`**: Material to assign to faces shaping the hexagons (white parts). Required if `material` is not set.
- **`pentagons_material`**: Material to assign to faces shaping the pentagons (black parts). Required if `material` is not set.
- **`subdivisions`**: Number of subdivision and projection steps. Each step nonuple(x9)s the face count. Default 2.

#### `cube_sphere`

A cube, subdivided and projected to a sphere, resulting in what's known as a [quadrilateralized spherical cube](https://en.wikipedia.org/wiki/Quadrilateralized_spherical_cube). Textured with a cube-map.

- **`material`**: Material to assign to every face. Required if `materials` isn't set. If set, faces share their UV space, so textures should contain the six cube-map faces in the following layout:
  
  | +x | +y | +z |
  |----|----|----|
  | -x | -y | -z |

- **`materials`**: An array of six materials for each face, in the order: +x, +y, +z, -x, -y, -z. Required if `material` isn't set. If set, each face has its independent UV space, so each face's texture is separate.
- **`subdivisions`**: The number of face rows and columns on each face.

## Materials

Materials define how the surface of meshes look. There are multiple types of material, and you can create instances of a type and use them in meshes.

> [!Warning]
> Do not pass materials directly to meshes. You have to use the method `as_material()` to convert it to the base type.

### Flags

Materials have several flags, which affect how the material is rendered, beyond their shading formula:

- `transparent`: Causes the faces with this flag to be rendered after those without, so the content behind them is available to the material. Has a small performance impact with forward mode, and big impact with deferred mode. Not needed for alpha cutout.
- `double_sided`: Used for flat meshes without a volume inside, like paper, glass, foliage, etc. Disables back-face culling.
- `alpha_cutout`: Enables alpha cutout or alpha testing, which means fragments with a base color alpha < 0.5 are not rendered. Useful in conjunction with `double_sided` and for foliage, fences, grates, etc.

### Normal maps

Used to add detail that would otherwise require a lot of polygons.

- **`normal_map`** (vector texture): Modifies surface normals to emulate surface detail. Uses DirectX style.
<!--
POM parameter controls Parallax [Occlusion] Mapping:

- `-1`: Disable
- `0`: Parallax Mapping
- `>0`: Enable Parallax Occlusion Mapping, parameter controls steps. It is advised that the lowest amount that looks correct at high angles be used. A typical value is 5.
-->
### `PhongMaterial`

Uses an expanded version of [Phong reflection model](https://en.wikipedia.org/wiki/Phong_reflection_model) to shade the materials.
This is the most flexible material but not as physically accurate as `PBRMaterial`.

```lua
my_mat = PhongMaterial.new{
    name= "Material",
    ...
}:as_material()
```

- **`diffuse`** (color texture): The base color. Proportional to the cosine of the angle between the light and the surface normal. Alpha is only used for alpha cutout (see flags).
- **`specular`** (color texture): The color of specular highlights. Alpha = 0 disables specular highlights, otherwise alpha is `10 * log2(shininess)`
- **`emissive`** (color texture): This is added to the lighting calculation regardless of incoming light, as if the material emits this light itself.
- **`tint`** (color texture): Depends on whether the material is transparent:
  - **Transparent materials**: Filters light coming from behind the material (the existing pixels).  
    Alpha is ignored. Useful for tinted glass, for example.
  - **Non-transparent materials**: Controls subsurface scattering.  
    In other words, light hitting the back of a flat object creates diffuse lighting visible at the front.  
    Alpha controls how much the intensity depends on view direction. Most useful for leaves. Only works for flat materials.
- **`normal_map`** (vector texture): See normal maps.
- **`transparent`** (boolean): See flags.
- **`double_sided`** (boolean): See flags.
- **`alpha_cutout`** (boolean): See flags.

### `PBRMaterial`

Uses [physically based rendering](https://en.wikipedia.org/wiki/Physically_based_rendering).
It is more accurate than `PhongMaterial` but slower. It uses the metallic-roughness workflow, Cook-Torrance BRDF, Fresnel-Schlick approximation, GGX normal distribution function, Smith geometry function with with Schlick-GGX approximation, energy conservation and Lambertian diffuse. (if you don't know what this means, don't worry, its good PBR sir)

```lua
my_mat = PBRMaterial.new{
    name= "Material",
    ...
}:as_material()
```

- **`albedo`** (color texture): The base color. Alpha is only used for alpha cutout (see flags).
- **`metallic`** (float texture): 0 for non-metallic, 1 for metallic. Metals have a much stronger base specular reflection and no diffuse reflection. Values between 0 and 1 are only intended for texture filtering.
- **`roughness`** (float texture): Defines how rough the surface is. The lower, the value, the sharper reflections are.
- **`ambient_occlusion`** (float texture): Controls how strong ambient lighting is. A value of 1 means 100% ambient lighting and 0 means no ambient lighting.
- **`transparent`** (boolean): See flags.
- **`double_sided`** (boolean): See flags.
- **`alpha_cutout`** (boolean): See flags.

### `EarthMaterial`

A specialized material to render the Earth. Uses a different material for terrain and ocean, based on an ocean mask. Overlays a cloud texture on top.

```lua
my_mat = EarthMaterial.new{
    name= "Material",
    ...
}:as_material()
```

- **`terrain_diffuse`** (color texture): Diffuse map for the terrain material. Can be obtained from NASA.
- **`city_lights`** (color texture): Emissive map for the terrain material. Can be obtained from NASA.
- **`normal_map`** (vector texture): Normal map for the terrain material. Hard to find.
- **`ocean_diffuse`** (color texture): Diffuse map for the ocean material. Use a solid blue.
- **`ocean_specular`** (color texture): Specular map for the ocean material. Use a solid light blue.
- **`ocean_mask`** (float texture): Used to determine which material to use. 1 for ocean, 0 for terrain.
- **`cloud_diffuse`** (color texture): Diffuse map for the cloud material. Use a solid white.
- **`cloud_texture`** (float texture): Controls cloud opacity. 1 for full cloud and 0 for no cloud. Can be obtained from NASA.

> [!Note]
> Earth material does not support flags.

## Textures

Textures convert UV coordinates to a value, be it a color, a vector or a float.

> [!Warning]
> Do not pass textures directly to materials. You have to use the method `as_texture()` to convert it to the base type.

### `SolidColorTexture`, `SolidVectorTexture`, `SolidFloatTexture`

Outputs the same value regardless of UV coordinates. Can be used to give a uniform property to a material, like a phong material that has blue diffuse everywhere, or a PBR material that is metallic everywhere.

```lua
texture_1 = SolidColorTexture.new{0.1, 0.2, 0.3, 0.4}:as_texture()
texture_2 = SolidVectorTexture.new{1, 2, 3}:as_texture() -- Not really useful
texture_3 = SolidFloatTexture.new(0.2):as_texture()
```

### `ImageColorTexture`, `ImageVectorTexture`, `ImageFloatTexture`

Reads values from an image file. The texel to be read depends on the UV:

- Top left is (0, 0)
- Top right is (0, 1)
- Bottom left is (1, 0)
- Bottom right is (1, 1)
- Any position in between is (0-1, 0-1)

The constructor takes two arguments:

1. File path, relative to caller script
2. Optional: A value to be multiplied with every texel

```lua
texture_1 = SolidColorTexture.new("./wood.png"):as_texture()
texture_1 = SolidColorTexture.new("./wood.png", {0.1, 0.2, 0.3, 0.4}):as_texture()
texture_2 = SolidVectorTexture.new("./wood-normalmap.png"):as_texture()
texture_2 = SolidVectorTexture.new("./wood-normalmap.png", {1, 2, 3}):as_texture()
texture_3 = SolidFloatTexture.new("./wood-heightmap.png"):as_texture()
texture_3 = SolidFloatTexture.new("./wood-heightmap.png", 0.2):as_texture()
```

Image pixels are interpreted differently depending on texture data type:

- `ImageColorTexture`: Pixels are used as is, only normalized to 0-1.
- `ImageVectorTexture`: R is X, G is Y, B is Z. A is unused.
- `ImageFloatTexture`: A is used as value. Other components are unused.

### `TinyImageTexture` (color only)

This texture behaves the same as `ImageColorTexture` but with the following differences:

- Data is kept as int8 and converted to float32 as texels are sampled, rather than storing in float32.
- No mipmaps are generated

This reduces memory consumption by about 16x and reduces loading times, which can be very useful for high resolution textures. It is also slightly faster, but the lack of mipmaps causes aliasing at higher densities.

### `SineWaveTexture` (float only)

This texture calculates the below formula:

$output = a\sin(bx+ct+d) + e$

Where:

- $a$, $b$, $c$, $d$ and $e$ are texture parameters
- $t$ is time elapsed
- $x$ is UV coordinates (either U or V is used depending on `orientation`)
  - If `orientation` is false, U is used
  - If `orientation` is true, V is used

```lua
my_texture = SineWaveTexture.new(a, b, c, d, e, orientation):as_texture()
my_texture = SineWaveTexture.new(0.5, 500, 5, 0, 0.5, true):as_texture()
```

### `BlendColorColorTexture`, `BlendColorFloatTexture`, `BlendVectorFloatTexture`, `BlendFloatFloatTexture`

Does blending between two textures. Has several modes:

- `alpha_mix`: interpolates between two colors by the second color's alpha.
- `add`: adds two values together
- `subtract`: subtracts the second value from the first value
- `multiply`: multiplies two values together

Depending on the data types, not all modes are available:

- Color & Color: `alpha_mix`, `add`, `subtract`, `multiply`
- Color & Float: `multiply`
- Vector & Float: `multiply`
- Float & Float: `add`, `subtract`, `multiply`

```lua
texture_1 = ImageColorTexture.new("./earth.png", {0, 1, 0}):as_texture()
texture_2 = SineWaveTexture.new(0.5, 500, 5, 0, 0.5, true):as_texture()
texture_mixed = BlendImageFloatTexture(texture_1, "multiply", texture_2):as_texture()
```

## Scripting

### `on_frame`

Calls a function every frame and passes it the time passed, in seconds.

```lua
on_frame(function(dt)
    if dt > 0.016 then
        print("We droppin' frames mate!")
    end
end)
```

### `is_key_pressed`

Determines if a keyboard key is currently being held.

```lua
on_frame(function(dt)
    if is_key_pressed(key.space) then
        jump()
    end
end)
```

For a list of key names, consult [SFML documentation](https://www.sfml-dev.org/documentation/3.0.2/namespacesf_1_1Keyboard.html#acb4cacd7cc5802dec45724cf3314a142). All key names have been converted to snake_case.
