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
- Phong shading  
- Phong reflection model support with additions: diffuse, specular and emissive  
- Alpha testing  
- Transparency support  
- Flat material support  
- Simple subsurface scattering for flat materials  
- Directional lights, point lights, spotlights, and ambient lighting  
- Render options: view framebuffer or z buffer, show wireframe, disable lighting (fullbright)  
- Normal mapping with adjustable strength  
- Texture filtering: nearest / bilinear / trilinear  
- Screen space fog based on Z buffer, with exponential falloff  
- Earth material, combines terrain, ocean and clouds  
- GUI allows almost all scene data to be controlled  
- Custom text file format for scene data
