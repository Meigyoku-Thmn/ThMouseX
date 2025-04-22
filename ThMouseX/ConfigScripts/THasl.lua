-- Runtime = Unity Mono
-- Script to get player's position in Abyss Soul Lotus
local camera = nil
local currentInstance = nil

local function CharacterUpdate(__instance: object): void
    local Camera = clr.UnityEngine.Camera
    if currentInstance ~= __instance then
        currentInstance = __instance
        camera = Camera.main
    end
    local position = camera:WorldToScreenPoint(__instance.transform.position)
    Position.X = position.x
    Position.Y = camera.pixelHeight - position.y - 1
end

return {
    --     Target method,      prehook,   posthook
    {'Centeryidong:FixedUpdate', nil, CharacterUpdate},
}