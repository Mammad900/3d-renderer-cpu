# CPU 3D renderer

This is a 3D renderer engine written from scratch and only uses the CPU. The only libraries used are the standard library, SFML for window creation and Dear ImGui for the GUI.

## Features

- Perspective projection
- Forward and deferred shading
- Multithreaded deferred pass
- Adjustable camera settings
- Instancing: create a mesh and reuse it with different scale, position and rotation
- Object tree system, objects have transforms that propagate to their children and contain components such as meshes, lights, cameras, etc.
- Each face can have its own material
- Flexible material system, works like shaders
- Phong shading (per pixel lighting)
- Alpha testing
- Transparency support
- Flat material support
- Simple subsurface scattering for flat materials
- Directional lights, point lights, spotlights, and ambient lighting
- Shadow mapping for spotlights
- God rays (volumetric lighting)
- Screen space fog based on Z buffer, with exponential falloff
- Fog and volumetric lighting with effects for transparent materials such as glass and fluids
- Skyboxes
- OBJ and STL file importer
- Mesh generators:

  - UV Sphere
  - Plane
  - Regular icosahedron
  - Subdivided icosahedron
  - Regular dodecahedron
  - Pentakis dodecahedron
  - Truncated icosahedron
  - Ball (subdivided truncated icosahedron)
  - Cube-sphere (quadrilateralized spherical cube)
- Material types:

  - Phong: Phong reflection model with additions: diffuse, specular and emissive
  - Earth: Combines terrain, ocean and clouds
  - PBR (Physically Based Rendering)
- Texture types:

  - Solid
  - Image (loads from image file)
  - Tiny Image (low-memory, no mipmaps)
  - Sine wave
  - Blend (combine multiple textures)
- Texture filtering: nearest / bilinear / trilinear
- Normal mapping with adjustable strength
- Render options: view framebuffer or z buffer, show wireframe, disable lighting (fullbright)
- GUI allows most scene data to be controlled
- Lua integration for scene definition and scripting
- Instantly switch between multiple scenes
- Offline rendering mode (pause real-time rendering and render one frame manually)
- Performance metrics with per-pass timing measurements
