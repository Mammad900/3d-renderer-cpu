# CPU 3D renderer

This is a 3D renderer engine written from scratch and only uses the CPU. The only libraries used are the standard library, SFML for window creation, Dear ImGui for the GUI and Sol3 for Lua integration.

Check out some of the best [renders](renders/renders.md).

## Features

- Perspective and orthographic projection
- Forward and deferred shading
- Multithreaded deferred pass
- Adjustable camera settings
- Instancing: create a mesh and reuse it with different scale, position and rotation
- Object tree system, objects have transforms that propagate to their children and contain components such as meshes, lights, cameras, etc.
- Each face can have its own material
- Flexible material system, works like shaders
- Phong shading (per pixel lighting)
- Alpha testing
- Order Independent Transparency
- Flat material support (no volume)
- Simple subsurface scattering for flat materials (aka back lighting)
- Directional lights, point lights, spotlights, and ambient lighting
- Shadow mapping
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
  - Extrude
- Material types:
  - Phong: Improved Phong reflection model: diffuse, specular, emissive, transmission and simple subsurface scattering
  - Earth: Combines terrain, ocean and clouds
  - PBR (Physically Based Rendering): Metallic-roughness workflow, fresnel effect, conservation of energy, IBL, etc.
- Texture types:
  - Solid
  - Image (loads from image file)
  - Tiny Image (low-memory, no mipmaps)
  - Sine wave
  - Blend (combine multiple textures)
- Environment maps (used for sky-box and reflections):
  - Solid color
  - Equirectangular / Panorama
  - Cube map
  - Infinite floor
  - Cubic room
- Texture filtering: nearest / bilinear / trilinear
- Normal mapping with adjustable strength
- Render options:
  - View frame-buffer, z-buffer or g-buffer
  - Show wireframe
  - Disable lighting (fullbright)
- GUI allows most scene data to be controlled
- Lua integration for scene definition and scripting
  - Lua REPL
- Instantly switch between multiple scenes
- Offline rendering mode (pause real-time rendering and render one frame manually)
  - Post processing when scripted
    - Bloom
    - Blur
    - Refraction
    - Tone-map to SDR
  - HDR or SDR output
- Performance metrics with per-pass timing measurements
- Multi-window with each window rendering its own camera and scene
- Spatial and non-spatial audio