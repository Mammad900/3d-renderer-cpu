---Like ImGui.ColorEdit3, but works with Color structs directly. Does not expose alpha, keeps it as is.
---@param label string
---@param value ColorT
---@return ColorT
function ImGui.ColorEdit3P(label, value)
    local r = ImGui.ColorEdit3(label, {value.r, value.g, value.b}, ImGuiColorEditFlags.Float | ImGuiColorEditFlags.HDR)
    return Color.new(r[1], r[2], r[3], value.a)
end

---Like ImGui.ColorEdit3P, with a separate alpha slider (from 0 to 1).
---@param label string Main control label
---@param label_a string Alpha control label
---@param value ColorT
---@return ColorT
function ImGui.ColorEdit3A(label, label_a, value)
    local r = ImGui.ColorEdit3(label, {value.r, value.g, value.b}, ImGuiColorEditFlags.Float | ImGuiColorEditFlags.HDR)
    local s = ImGui.SliderFloat(label_a, value.a, 0, 1)
    return Color.new(r[1], r[2], r[3], s)
end

---Creates a window of given width positioned at the top right corner, and cannot be moved or resized. Don't forget End()!
---@param label string
---@param width integer
---@param window WindowT
function ImGui.CornerWindow(label, width, window)
    ImGui.SetNextWindowSize(width, 0, ImGuiCond.Always)
    local ws_x = window.size[1]
    ImGui.SetNextWindowPos(ws_x - 10, 10, ImGuiCond.Always, 1, 0)
    return ImGui.Begin(label)
end