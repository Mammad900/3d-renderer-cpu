---@meta

-- This file is usually more up to date than LuaApi.md, 
-- since I need to update it before I can use any new API myself. 
-- However, it lacks descriptions.

-- Types ending with T indicate the runtime type. 
-- For example, Color.new() constructs a runtime Color object which corresponds to ColorT. 
-- Never construct one yourself with tables, always use the corresponding .new() constructor.

-- Types ending with C indicate constructor tables. 
-- For example, Color.new() takes a ColorC as its parameter.

-- Types ending with CC indicate what type can be used in another constructor that accepts the type. 
-- For example, VertexC.position is a ColorCC. 

--#region Color

---@class ColorT
---@field r number Red
---@field g number Green
---@field b number Blue
---@field a number Alpha
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class ColorC
---@field r? number Red
---@field g? number Green
---@field b? number Blue
---@field a? number Alpha

---@alias ColorCC ColorT | number | [number?,number?,number?,number?]

Color = {}

---@overload fun(t: ColorC): ColorT
---@overload fun(r: number, g: number, b: number, a: number): ColorT
function Color.new() end

--#endregion
--#region Vec3

---@class Vec3T
---@field x number
---@field y number
---@field z number
---@field normalized fun(self: Vec3T): Vec3T
---@field length fun(self: Vec3T): number
---@operator add(Vec3T): Vec3T
---@operator sub(Vec3T): Vec3T
---@operator mul(number): Vec3T
---@operator div(number): Vec3T
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class Vec3C
---@field x? number
---@field y? number
---@field z? number

---@alias Vec3CC Vec3T | number | [number?,number?,number?]

Vec3 = {}

---@overload fun(t: Vec3C): Vec3T
---@overload fun(x: number, y: number, z: number): Vec3T
function Vec3.new() end

---@param a Vec3T
---@param b Vec3T
---@return number
function Vec3.dot(a, b) end

---@param a Vec3T
---@param b Vec3T
---@return Vec3T
function Vec3.cross(a, b) end

---@param deg Vec3CC
---@return Vec3T
function deg_to_rad(deg) end

---@param rad Vec3CC
---@return Vec3T
function rad_to_deg(rad) end

---@alias Vec2CC number | [number|number]

--#endregion
--#region Volume

---@class VolumeT
---@field diffuse ColorT
---@field emissive ColorT
---@field transmission ColorT
---@field god_rays boolean
---@field god_rays_sample_size number
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class VolumeC
---@field diffuse? ColorCC
---@field emissive? ColorCC
---@field transmission? ColorCC
---@field god_rays? boolean
---@field god_rays_sample_size? number

Volume = {}

---@param props VolumeC
---@return VolumeT
function Volume.new(props) end


--#endregion
--#region Scene
---@alias TextureFilteringMode
---| "nearest_neighbor"
---| "bilinear"
---| "trilinear"

---@class SceneT
---@field name string
---@field sky_box EnvironmentMapT
---@field objects ObjectList
---@field add_object fun(self: SceneT, object: ObjectT)
---@field add_objects fun(self: SceneT, object: ObjectT[])
---@field back_face_culling boolean
---@field ambient_light ColorT
---@field volume VolumeT
---@field bilinear_shadow_filtering boolean
---@field shadow_bias number
---@field wire_frame boolean
---@field full_bright boolean
---@field always_update boolean
---@field texture_filtering_mode TextureFilteringMode
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

Scene = {}

---@return SceneT
function Scene.new() end

--#endregion
--#region Window

---@class WindowT
---@field close fun(self: WindowT)
---@field size [number, number]
---@field frame_size [number, number]
---@field tool_window_for? WindowT
---@field has_gui boolean
---@field quit_when_closed boolean
---@field sync_frame_size boolean
---@field name string
---@field camera CameraT
---@field scene SceneT
---@field set_camera fun(self: WindowT, scene: SceneT, camera: CameraT)
---@field remove_camera fun(self: WindowT)
---@field deferred boolean
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class WindowC
---@field size [number, number]
---@field quit_when_closed? boolean
---@field camera? CameraT
---@field deferred? boolean
---@field scene? SceneT
---@field tool_window_for? WindowT
---@field has_gui? boolean
---@field name? string
---@field sync_frame_size? boolean
---@field on_gui? fun()
---@field on_event? fun(ev: Event)

Window = {}

---@param props WindowC
---@return WindowT
function Window.new(props) end


---@class EventResized
---@field type "resized"
---@field x number
---@field y number

---@class EventKeyPressed
---@field type "key_pressed"
---@field key integer
---@field alt boolean
---@field ctrl boolean
---@field super boolean
---@field shift boolean

---@class EventKeyReleased
---@field type "key_released"
---@field key integer
---@field alt boolean
---@field ctrl boolean
---@field super boolean
---@field shift boolean

---@class EventMouseMoved
---@field type "mouse_moved"
---@field position_x number
---@field position_y number

---@class EventMouseMovedRaw
---@field type "mouse_moved_raw"
---@field delta_x number
---@field delta_y number

---@class EventMouseButtonPressed
---@field type "mouse_button_pressed"
---@field position_x number
---@field position_y number
---@field button integer

---@class EventMouseButtonReleased
---@field type "mouse_button_released"
---@field position_x number
---@field position_y number
---@field button integer

---@alias Event EventResized | EventKeyPressed | EventKeyReleased | EventMouseMovedRaw | EventMouseMovedRaw | EventMouseButtonPressed | EventMouseButtonReleased

--#endregion
--#region Object

---@class ObjectList
---@field size fun(self: ObjectList): number
---@field at fun(self: ObjectList, i: number): ObjectT # Index starts at 0
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class ComponentList
---@field size fun(self: ComponentList): number
---@field at fun(self: ComponentList, i: number): Component # Index starts at 0
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class ObjectT
---@field children ObjectList
---@field components ComponentList
---@field name string
---@field position Vec3T
---@field rotation Vec3T
---@field scale Vec3T
---@field add_child fun(self: ObjectT, child: ObjectT)
---@field add_component fun(self: ObjectT, component: Component)
---@field transform fun(self: ObjectT, vec: Vec3T): Vec3T
---@field transform_normal fun(self: ObjectT, vec: Vec3T): Vec3T
---@field transform_rotation fun(self: ObjectT, vec: Vec3T): Vec3T
---@field update_transform fun(self: ObjectT)
---@field global_position Vec3T
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class ObjectC
---@field name? string
---@field position? Vec3CC
---@field rotation? Vec3CC
---@field rotation_order? "xyz"|"zyx"
---@field scale? Vec3CC
---@field children? ObjectT[]
---@field components? Component[]

Object = {}

---@param props ObjectC
---@return ObjectT
function Object.new(props) end

--#endregion
--#region Component

---@class Component
---@field object ObjectT
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class MeshComponentT
---@field mesh MeshT
---@field as_component fun(self: MeshComponentT): Component
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

MeshComponent = {}

---@param mesh MeshT
---@return MeshComponentT
function MeshComponent.new(mesh) end

---@class RotatorComponentT
---@field as_component fun(self: RotatorComponentT): Component
---@field rotate_per_second Vec3T
---@field enabled boolean
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

RotatorComponent = {}

---@param props Vec3CC
---@return RotatorComponentT
function RotatorComponent.new(props) end

---@class ScriptComponentT
---@field as_component fun(self: ScriptComponentT): Component
---@field object ObjectT
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class ScriptComponentC
---@field update? fun(dt: number, comp: ScriptComponentT)
---@field pre_update? fun(dt: number, comp: ScriptComponentT)
---@field gui? fun(comp: ScriptComponentT)
---@field name? string | fun(comp: ScriptComponentT):string

ScriptComponent = {}

---@param props ScriptComponentC
---@return ScriptComponentT
function ScriptComponent.new(props) end

--#endregion
--#region Light

---@class PointLightT
---@field color ColorT
---@field as_component fun(self: PointLightT): Component
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class PointLightC
---@field color ColorCC
---@field shadow_map? {size: Vec2CC, near?: number, far?: number}

PointLight = {}

---@param color ColorT | PointLightC
---@return PointLightT
function PointLight.new(color) end


---@class DirectionalLightT
---@field color ColorT
---@field as_component fun(self: DirectionalLightT): Component
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class DirectionalLightC
---@field color ColorCC
---@field shadow_map? {size: Vec2CC, fov: number, near?: number, far?: number}

DirectionalLight = {}

---@param color ColorT | DirectionalLightC
---@return DirectionalLightT
function DirectionalLight.new(color) end


---@class SpotLightT
---@field color ColorT
---@field spread_inner number
---@field spread_outer number
---@field as_component fun(self: SpotLightT): Component
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class SpotLightC
---@field color ColorCC
---@field spread_inner number
---@field spread_outer number
---@field shadow_map? {size: Vec2CC, near?: number, far?: number}

SpotLight = {}

---@param color ColorT
---@param spread_inner number
---@param spread_outer number
---@return SpotLightT
---@overload fun(props: SpotLightC): SpotLightT
function SpotLight.new(color, spread_inner, spread_outer) end

--#endregion
--#region Camera

---@class CameraT
---@field fov number
---@field near number
---@field far number
---@field white_point number
---@field as_component fun(self: CameraT): Component

---@class CameraC
---@field fov? number
---@field near? number
---@field far? number
---@field white_point? number

Camera = {}

---@param props CameraC
---@return CameraT
function Camera.new(props) end

--#endregion
--#region Mesh

---@class VertexT
---@field position Vec3T
---@field normal Vec3T
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class VertexC
---@field position? Vec3CC
---@field uv? [number, number]
---@field normal? Vec3CC

Vertex = {}

---@param props VertexC
---@return VertexT
function Vertex.new(props) end

---@class FaceT
---@field v1 integer
---@field v2 integer
---@field v3 integer
---@field material Material
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

Face = {}

---@param props FaceT
---@return FaceT
---@overload fun(v1: integer, v2: integer, v3: integer, material: Material): FaceT
function Face.new(props) end

---@class MeshT
---@field label string
---@field flatShading boolean
---@field vertex_count integer
---@field face_count integer
---@field vertex_at fun(self: MeshT, i: integer): VertexT
---@field face_at fun(self: MeshT, i: integer): FaceT
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF"

---@class MeshC
---@field label? string
---@field flat_shading? string
---@field auto_normals? string
---@field vertices? (VertexT | VertexC)[]
---@field faces? (FaceT | [integer, integer, integer, Material])[]

Mesh = {}

---@param props MeshC
---@return MeshT
function Mesh.new(props) end

--#endregion
--#region Generate Mesh

---@class generate_mesh_sphere
---@field type "sphere"
---@field material Material
---@field name? string
---@field sectors? integer
---@field invert_u? boolean
---@field invert_v? boolean
---@field flat_shading? boolean

---@class generate_mesh_cylinder
---@field type "cylinder"
---@field material Material
---@field name? string
---@field sectors? integer
---@field end_cap? boolean
---@field end_cap_material? Material
---@field start_cap? boolean
---@field start_cap_material? Material
---@field flat_shading? boolean

---@class generate_mesh_plane
---@field type "plane"
---@field material Material
---@field name? string
---@field subdivisions_x? integer
---@field subdivisions_y? integer
---@field flat_shading? boolean

---@class generate_mesh_extrude
---@field type "extrude"
---@field material Material
---@field name? string
---@field vertices [number, number][]
---@field end_cap? boolean
---@field end_cap_material? Material
---@field start_cap? boolean
---@field start_cap_material? Material

---@class generate_mesh_obj
---@field type "obj"
---@field material Material
---@field name? string
---@field file string
---@field flat_shading? boolean

---@class generate_mesh_stl
---@field type "stl"
---@field material Material
---@field name? string
---@field file string
---@field flat_shading? boolean

---@class generate_mesh_regular_icosahedron
---@field type "regular_icosahedron"
---@field material Material
---@field name? string
---@field flat_shading? boolean

---@class generate_mesh_ico_sphere
---@field type "ico_sphere"
---@field material Material
---@field name? string
---@field subdivisions? integer
---@field flat_shading? boolean

---@class generate_mesh_dodecahedron
---@field type "dodecahedron"
---@field material Material
---@field name? string
---@field is_pentakis? boolean
---@field flat_shading? boolean

---@class generate_mesh_ball
---@field type "dodecahedron"
---@field material? Material
---@field hexagons_material? Material
---@field pentagons_material? Material
---@field name? string
---@field subdivisions? integer
---@field flat_shading? boolean

---@class generate_mesh_cube
---@field type "cube" | "cube_sphere"
---@field material? Material
---@field materials? [Material, Material, Material, Material, Material, Material]
---@field name? string
---@field subdivisions? integer
---@field flat_shading? boolean

---@alias generate_mesh_all generate_mesh_sphere | generate_mesh_cylinder | generate_mesh_plane | generate_mesh_extrude | generate_mesh_obj | generate_mesh_stl | generate_mesh_regular_icosahedron | generate_mesh_ico_sphere | generate_mesh_dodecahedron | generate_mesh_ball | generate_mesh_cube

-- -@generic T: generate_mesh_all
---@param props generate_mesh_all
---@return MeshT
function generate_mesh(props) end

--#endregion
--#region Material

---@class Material
---@field transparent boolean
---@field double_sided boolean
---@field alpha_cutout boolean
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_MATERIAL"



---@class PhongMaterialT
---@field diffuse ColorTexture
---@field specular ColorTexture
---@field tint ColorTexture
---@field emissive ColorTexture
---@field normal_map VectorTexture
---@field volume_front VolumeT
---@field volume_back VolumeT
---@field environment_reflection ColorT
---@field as_material fun(self: PhongMaterialT): Material
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_PHONG"

---@class PhongMaterialC
---@field name? string
---@field diffuse? ColorTexture
---@field specular? ColorTexture
---@field tint? ColorTexture
---@field emissive? ColorTexture
---@field normal_map? VectorTexture
---@field environment_reflection? ColorCC
---@field transparent? boolean
---@field double_sided? boolean
---@field alpha_cutout? boolean
---@field volume_front? VolumeT
---@field volume_back? VolumeT

PhongMaterial = {}

---@param props PhongMaterialC
---@return PhongMaterialT
function PhongMaterial.new(props) end



---@class EarthMaterialT
---@field terrain_diffuse ColorTexture
---@field city_lights ColorTexture
---@field ocean_diffuse ColorTexture
---@field ocean_specular ColorTexture
---@field ocean_mask FloatTexture
---@field cloud_texture FloatTexture
---@field as_material fun(self: EarthMaterialT): Material
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_EARTH"

---@class EarthMaterialC
---@field name? string
---@field terrain_diffuse? ColorTexture
---@field city_lights? ColorTexture
---@field normal_map? VectorTexture
---@field ocean_diffuse? ColorTexture
---@field ocean_specular? ColorTexture
---@field ocean_mask? FloatTexture
---@field cloud_diffuse? ColorTexture
---@field cloud_texture? FloatTexture

EarthMaterial = {}

---@param props EarthMaterialC
---@return EarthMaterialT
function EarthMaterial.new(props) end



---@class PBRMaterialT
---@field albedo ColorTexture
---@field metallic FloatTexture
---@field roughness FloatTexture
---@field ambient_occlusion FloatTexture
---@field as_material fun(self: PBRMaterialT): Material
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_PBR"

---@class PBRMaterialC
---@field name? string
---@field albedo? ColorTexture
---@field metallic? FloatTexture
---@field roughness? FloatTexture
---@field ambient_occlusion? FloatTexture
---@field transparent? boolean
---@field double_sided? boolean
---@field alpha_cutout? boolean

PBRMaterial = {}

---@param props PBRMaterialC
---@return PBRMaterialT
function PBRMaterial.new(props) end

--#endregion
--#region Texture

---@class ColorTexture
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_COLOR"

---@class VectorTexture
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_VECTOR"

---@class FloatTexture
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_FLOAT"



---@class SolidColorTextureT
---@field value ColorT
---@field as_texture fun(self: SolidColorTextureT): ColorTexture

SolidColorTexture = {}

---@param color ColorCC
---@return SolidColorTextureT
function SolidColorTexture.new(color) end


---@class SolidVectorTextureT
---@field value Vec3T
---@field as_texture fun(self: SolidVectorTextureT): VectorTexture

SolidVectorTexture = {}

---@param vec Vec3CC
---@return SolidVectorTextureT
function SolidVectorTexture.new(vec) end


---@class SolidFloatTextureT
---@field value number
---@field as_texture fun(self: SolidFloatTextureT): FloatTexture

SolidFloatTexture = {}

---@param num number
---@return SolidFloatTextureT
function SolidFloatTexture.new(num) end



---@class ImageColorTextureT
---@field scale ColorT
---@field as_texture fun(self: ImageColorTextureT): ColorTexture
---@field save_to_file fun(self: ImageColorTextureT, path: string): boolean

ImageColorTexture = {}

---@param path string
---@param scale? ColorCC
---@return ImageColorTextureT
function ImageColorTexture.new(path, scale) end


---@class ImageVectorTextureT
---@field scale Vec3T
---@field as_texture fun(self: ImageVectorTextureT): VectorTexture
---@field save_to_file fun(self: ImageVectorTextureT, path: string): boolean

ImageVectorTexture = {}

---@param path string
---@param scale? Vec3CC
---@return ImageVectorTextureT
function ImageVectorTexture.new(path, scale) end


---@class ImageFloatTextureT
---@field scale number
---@field as_texture fun(self: ImageFloatTextureT): FloatTexture
---@field save_to_file fun(self: ImageFloatTextureT, path: string): boolean

ImageFloatTexture = {}

---@param path string
---@param scale? number
---@return ImageFloatTextureT
function ImageFloatTexture.new(path, scale) end



---@class SliceColorTextureT
---@field texture ColorTexture
---@field as_texture fun(self: SliceColorTextureT): ColorTexture
---@field save_to_file fun(self: SliceColorTextureT, path: string): boolean

SliceColorTexture = {}

---@param texture ColorTexture
---@param scale [number, number]
---@param offset [number, number]
---@return SliceColorTextureT
function SliceColorTexture.new(texture, scale, offset) end

---@param texture ColorTexture
---@param n [integer, integer]
---@return SliceColorTextureT[]
function slice_color_texture(texture, n) end


---@class SliceVectorTextureT
---@field texture VectorTexture
---@field as_texture fun(self: SliceVectorTextureT): VectorTexture
---@field save_to_file fun(self: SliceVectorTextureT, path: string): boolean

SliceVectorTexture = {}

---@param texture VectorTexture
---@param scale [number, number]
---@param offset [number, number]
---@return SliceVectorTextureT
function SliceVectorTexture.new(texture, scale, offset) end

---@param texture VectorTexture
---@param n [integer, integer]
---@return SliceVectorTextureT[]
function slice_vector_texture(texture, n) end


---@class SliceFloatTextureT
---@field texture FloatTexture
---@field as_texture fun(self: SliceFloatTextureT): FloatTexture
---@field save_to_file fun(self: SliceFloatTextureT, path: string): boolean

SliceFloatTexture = {}

---@param texture FloatTexture
---@param scale [number, number]
---@param offset [number, number]
---@return SliceFloatTextureT
function SliceFloatTexture.new(texture, scale, offset) end

---@param texture FloatTexture
---@param n [integer, integer]
---@return SliceFloatTextureT[]
function slice_float_texture(texture, n) end


---@class TinyImageTextureT
---@field scale ColorT
---@field as_texture fun(self: TinyImageTextureT): ColorTexture
---@field save_to_file fun(self: TinyImageTextureT, path: string): boolean

TinyImageTexture = {}

---@param path string
---@param scale? ColorCC
---@return TinyImageTextureT
function TinyImageTexture.new(path, scale) end

---@class SineWaveTextureT
---@field a number
---@field b number
---@field c number
---@field d number
---@field e number
---@field orientation boolean
---@field as_texture fun(self: SineWaveTextureT): FloatTexture

SineWaveTexture = {}

---@param a number
---@param b number
---@param c number
---@param d number
---@param e number
---@param orientation boolean
---@return SineWaveTextureT
function SineWaveTexture.new(a, b, c, d, e, orientation) end




---@class BlendColorColorTextureT
---@field a ColorTexture
---@field b ColorTexture
---@field mode "add" | "multiply" | "subtract" | "alpha_mix"
---@field as_texture fun(self: BlendColorColorTextureT): ColorTexture

BlendColorColorTexture = {}

---@param a ColorTexture
---@param b ColorTexture
---@param mode "add" | "multiply" | "subtract" | "alpha_mix"
---@return BlendColorColorTextureT
function BlendColorColorTexture.new(a, mode, b) end


---@class BlendColorFloatTextureT
---@field a ColorTexture
---@field b FloatTexture
---@field mode "multiply"
---@field as_texture fun(self: BlendColorFloatTextureT): ColorTexture

BlendColorFloatTexture = {}

---@param a ColorTexture
---@param b FloatTexture
---@param mode "multiply"
---@return BlendColorFloatTextureT
function BlendColorFloatTexture.new(a, mode, b) end


---@class BlendVectorFloatTextureT
---@field a VectorTexture
---@field b FloatTexture
---@field mode "multiply"
---@field as_texture fun(self: BlendVectorFloatTextureT): VectorTexture

BlendVectorFloatTexture = {}

---@param a VectorTexture
---@param b FloatTexture
---@param mode "multiply"
---@return BlendVectorFloatTextureT
function BlendVectorFloatTexture.new(a, mode, b) end


---@class BlendFloatFloatTextureT
---@field a FloatTexture
---@field b FloatTexture
---@field mode "add" | "multiply" | "subtract"
---@field as_texture fun(self: BlendFloatFloatTextureT): FloatTexture

BlendFloatFloatTexture = {}

---@param a VectorTexture
---@param b FloatTexture
---@param mode "add" | "multiply" | "subtract"
---@return BlendFloatFloatTextureT
function BlendFloatFloatTexture.new(a, mode, b) end


--#endregion
--#region EnvironmentMap

---@class EnvironmentMapT
---@field z__DO_NOT_CONSTRUCT_YOURSELF "DO_NOT_CONSTRUCT_YOURSELF_ENV"


---@class SolidEnvironmentMapT
---@field color ColorT
---@field as_environment_map fun(self: SolidEnvironmentMapT): EnvironmentMapT

SolidEnvironmentMap = {}

---@param value ColorCC
---@return SolidEnvironmentMapT
function SolidEnvironmentMap.new(value) end


---@class PanoramaMapT
---@field texture ColorTexture
---@field as_environment_map fun(self: PanoramaMapT): EnvironmentMapT

PanoramaMap = {}

---@param texture ColorTexture
---@return PanoramaMapT
function PanoramaMap.new(texture) end


---@class AtlasCubeMapT
---@field texture ColorTexture
---@field as_environment_map fun(self: AtlasCubeMapT): EnvironmentMapT

AtlasCubeMap = {}

---@param texture ColorTexture
---@return AtlasCubeMapT
function AtlasCubeMap.new(texture) end


---@class CubeMapT
---@field as_environment_map fun(self: CubeMapT): EnvironmentMapT

CubeMap = {}

---@param textures [ColorTexture, ColorTexture, ColorTexture, ColorTexture, ColorTexture, ColorTexture]
---@return CubeMapT
function CubeMap.new(textures) end

--#endregion
--#region Scripting

---@type table<any, fun(delta: number)>
on_frame= {}

---@param key number
---@return boolean
function is_key_pressed(key) end

key = {
    unknown= 0,
    a= 0,
    b= 0,
    c= 0,
    d= 0,
    e= 0,
    f= 0,
    g= 0,
    h= 0,
    i= 0,
    j= 0,
    k= 0,
    l= 0,
    m= 0,
    n= 0,
    o= 0,
    p= 0,
    q= 0,
    r= 0,
    s= 0,
    t= 0,
    u= 0,
    v= 0,
    w= 0,
    x= 0,
    y= 0,
    z= 0,
    num0= 0,
    num1= 0,
    num2= 0,
    num3= 0,
    num4= 0,
    num5= 0,
    num6= 0,
    num7= 0,
    num8= 0,
    num9= 0,
    escape= 0,
    l_control= 0,
    l_shift= 0,
    l_alt= 0,
    l_system= 0,
    r_control= 0,
    r_shift= 0,
    r_alt= 0,
    r_system= 0,
    menu= 0,
    l_bracket= 0,
    r_bracket= 0,
    semicolon= 0,
    comma= 0,
    period= 0,
    apostrophe= 0,
    slash= 0,
    backslash= 0,
    grave= 0,
    equal= 0,
    hyphen= 0,
    space= 0,
    enter= 0,
    backspace= 0,
    tab= 0,
    page_up= 0,
    page_down= 0,
    ["end"]= 0,
    home= 0,
    insert= 0,
    delete= 0,
    add= 0,
    subtract= 0,
    multiply= 0,
    divide= 0,
    left= 0,
    right= 0,
    up= 0,
    down= 0,
    numpad0= 0,
    numpad1= 0,
    numpad2= 0,
    numpad3= 0,
    numpad4= 0,
    numpad5= 0,
    numpad6= 0,
    numpad7= 0,
    numpad8= 0,
    numpad9= 0,
    f1= 0,
    f2= 0,
    f3= 0,
    f4= 0,
    f5= 0,
    f6= 0,
    f7= 0,
    f8= 0,
    f9= 0,
    f10= 0,
    f11= 0,
    f12= 0,
    f13= 0,
    f14= 0,
    f15= 0,
    pause= 0,
}

--#endregion
