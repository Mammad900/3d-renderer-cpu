---Adds a callback to on_frame, and returns a function to remove that callback
---@param cb fun(dt: number)
---@return fun()
local function add_frame_callback(cb)
    table.insert(on_frame, cb)
    local index = #on_frame
    return function()
        if on_frame[index] == cb then
            table.remove(on_frame, index)
        else
            for i, v in ipairs(on_frame) do
                if v == cb then
                    table.remove(on_frame, i); break
                end
            end
        end
    end
end

---Animates a value over a timeframe and calls a callback with the current value every frame
---@param start_val number
---@param end_val number
---@param duration number
---@param callback fun(val: number)
---@return fun()
local function animate(start_val, end_val, duration, callback)
    local elapsed = 0
    local cancel
    cancel = add_frame_callback(function(dt)
        elapsed = elapsed + dt
        local t = math.min(elapsed / duration, 1)
        local value = start_val + (end_val - start_val) * t
        callback(value)
        if t >= 1 then
            cancel()
        end
    end)
    return cancel
end

return {
    add_frame_callback = add_frame_callback,
    animate = animate
}
