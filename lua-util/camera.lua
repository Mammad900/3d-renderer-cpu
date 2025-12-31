
---@class make_camera_props: CameraC
---@field distance? number Camera orbit radius. Default 5.
---@field light? boolean Whether to add a white directional light of strength 0.5 shining forward.
---@field speed? number Speed of keyboard controls.

---Creates a ready to use Camera object orbiting the world origin, and with keyboard controls. Returns in order:
---1. Camera orbit object. Add this to the scene tree. Its rotation and position are keyboard controlled.
---2. Camera instance. Give it to window:set_camera.
---3. Camera object, which is nested inside camera orbit object. No need to do anything with it.
---4. The DirectionalLight component if created, otherwise nil. No need to do anything with it.
---5. A callback to set keyboard control speed.
---@param props2 make_camera_props
return function(props2)
    local props1 = {
        fov = 75,
        active = true,
        distance = 5,
        speed = 2
    }
    for k,v in pairs(props2) do props1[k] = v end

    local camera_comp = Camera.new(props1)
    print(props1.distance)
    local camera = Object.new{
        name= "Camera",
        position= {0, 0, -props1.distance},
        components= {
            camera_comp:as_component()
        }
    }
    local light = nil
    if props1.light then
        light = DirectionalLight.new(Color.new(1,1,1,0.5))
        camera:add_component(light:as_component())
    end
    local camera_orbit = Object.new{
        name= "Camera Orbit",
        rotation= props1.rotation,
        children= {camera}
    }
    camera_orbit:add_component(ScriptComponent.new{
        name= "Camera Control",
        gui= function ()
            ImGui.DragFloat("Speed", props1.speed, 1, 0.1, 50, "%.3f", ImGuiSliderFlags.Logarithmic);
        end,
        update = function(dt)
            dt = dt * props1.speed
            local move = Vec3.new(0, 0, 0)
            if is_key_pressed(key.q) then
                camera_orbit.rotation.z = camera_orbit.rotation.z + dt
            end
            if is_key_pressed(key.e) then
                camera_orbit.rotation.z = camera_orbit.rotation.z - dt
            end
            if is_key_pressed(key.right) then
                camera_orbit.rotation.y = camera_orbit.rotation.y + dt
            end
            if is_key_pressed(key.left) then
                camera_orbit.rotation.y = camera_orbit.rotation.y - dt
            end
            if is_key_pressed(key.up) then
                camera_orbit.rotation.x = camera_orbit.rotation.x + dt
            end
            if is_key_pressed(key.down) then
                camera_orbit.rotation.x = camera_orbit.rotation.x - dt
            end
            if is_key_pressed(key.a) then
                move.x = dt
            end
            if is_key_pressed(key.d) then
                move.x = -dt
            end
            if is_key_pressed(key.w) then
                move.z = dt
            end
            if is_key_pressed(key.s) then
                move.z = -dt
            end
            if is_key_pressed(key.r) then
                move.y = dt
            end
            if is_key_pressed(key.f) then
                move.y = -dt
            end
            if is_key_pressed(key.add) then
                camera.position.z = camera.position.z * (0.5^dt)
            end
            if is_key_pressed(key.subtract) then
                camera.position.z = camera.position.z * (0.5^-dt)
            end
            if is_key_pressed(key.multiply) then
                camera_comp.fov = camera_comp.fov * (0.5^-dt)
            end
            if is_key_pressed(key.divide) then
                camera_comp.fov = camera_comp.fov * (0.5^dt)
            end
            move = camera_orbit:transform_rotation(move)
            camera_orbit.position.x = camera_orbit.position.x + move.x * -0.2 * camera.position.z;
            camera_orbit.position.y = camera_orbit.position.y + move.y * -0.2 * camera.position.z;
            camera_orbit.position.z = camera_orbit.position.z + move.z * -0.2 * camera.position.z;
            if is_key_pressed(key.z) then
                camera_orbit.position = Vec3.new(0,0,0)
            end
        end
    }:as_component())

    return camera_orbit, camera_comp, camera, light, function (s) props1.speed = s end
end