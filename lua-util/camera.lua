
---@class make_camera_props: CameraC
---@field distance? number Camera orbit radius. Default 5.
---@field light? boolean Whether to add a white directional light of strength 0.5 shining forward.
---@field speed? number Speed of keyboard controls.
---@field rotation? Vec3T Initial rotation

---Creates a ready to use Camera object orbiting the world origin, and with keyboard controls. 
---
---Controls:
---- WASD: Move horizontally
---- R/F: Move vertically
---- LShift: Speed up movement by 3x
---- Arrow keys: Look pitch/yaw
---- Q/E: Roll
---- Num +-: Adjust orbit distance
---- Num */: Adjust FOV
---- Z: Reset
---
---Returns in order:
---1. Camera orbit object. Add this to the scene tree. Its rotation and position are keyboard controlled.
---2. Camera instance. Give it to window:set_camera.
---3. Camera object, which is nested inside camera orbit object. No need to do anything with it.
---4. The DirectionalLight component if created, otherwise nil. No need to do anything with it.
---5. A callback to set keyboard control speed.
---@param props2 make_camera_props
return function(props2)
    local props = {
        fov = 75,
        active = true,
        distance = 5,
        speed = 2,
        rotation= Vec3.new(0,0,0)
    }
    for k,v in pairs(props2) do props[k] = v end

    local camera_comp = Camera.new(props)
    local camera = Object.new{
        name= "Camera",
        position= {0, 0, -props.distance},
        components= {
            camera_comp:as_component()
        }
    }

    local light = nil
    if props.light then
        light = DirectionalLight.new(Color.new(1,1,1,0.5))
        camera:add_component(light:as_component())
    end

    local camera_orbit = Object.new{
        name= "Camera Orbit",
        rotation= props.rotation,
        children= {camera}
    }

    local is_orbit = true;

    camera_orbit:add_component(ScriptComponent.new{
        name= "Camera Control",
        gui= function ()
            props.speed = ImGui.DragFloat("Speed", props.speed, 1, 0.1, 50, "%.3f", ImGuiSliderFlags.Logarithmic);
        end,
        pre_update = function(dt)
            dt = dt * props.speed
            local move = Vec3.new(0, 0, 0)
            local rotate = Vec3.new(0, 0, 0)

            if is_key_pressed(key.q) then
                rotate.z = dt
            end
            if is_key_pressed(key.e) then
                rotate.z = -dt
            end
            if is_key_pressed(key.right) then
                rotate.y = dt
            end
            if is_key_pressed(key.left) then
                rotate.y = -dt
            end
            if is_key_pressed(key.up) then
                rotate.x = dt
            end
            if is_key_pressed(key.down) then
                rotate.x = -dt
            end
            if is_key_pressed(key.a) then
                move.x = 1
            end
            if is_key_pressed(key.d) then
                move.x = -1
            end
            if is_key_pressed(key.w) then
                move.z = 1
            end
            if is_key_pressed(key.s) then
                move.z = -1
            end
            if is_key_pressed(key.r) then
                move.y = 1
            end
            if is_key_pressed(key.f) then
                move.y = -1
            end
            if is_key_pressed(key.add) then
                camera.position.z = camera.position.z * (0.5^dt)
            end
            if is_key_pressed(key.subtract) then
                camera.position.z = camera.position.z * (0.5^-dt)
            end
            if is_key_pressed(key.multiply) then
                camera_comp.fov = math.min(170, camera_comp.fov * (0.5^-dt))
            end
            if is_key_pressed(key.divide) then
                camera_comp.fov = camera_comp.fov * (0.5^dt)
            end

            local y = move.y
            move = camera_orbit:transform_rotation(move)
            move.y = y -- Vertical movement isn't affected by look

            if(move:length() > 0) then
                move = move:normalized() * dt
                if is_key_pressed(key.l_shift) then
                    move = move * 3
                end
                if is_orbit then
                    move = move + camera.global_position - camera_orbit.global_position -- Trick to make the transition seamless
                    is_orbit = false
                end
                camera.position.z = 0
                camera_orbit.position = camera_orbit.position + move;
            end

            if not is_orbit then
                rotate = rotate * -1
            end
            camera_orbit.rotation = camera_orbit.rotation + rotate

            if is_key_pressed(key.z) then -- Reset everything
                camera_orbit.position = Vec3.new(0,0,0)
                camera_orbit.rotation = props.rotation
                camera.position.z = -props.distance
                is_orbit = true
            end
        end
    }:as_component())

    return camera_orbit, camera_comp, camera, light, function (s) props.speed = s end
end