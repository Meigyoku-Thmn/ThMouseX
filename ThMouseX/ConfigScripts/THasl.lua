-- Runtime = Unity Mono
-- Script to get player's position in Abyss Soul Lotus
local function CharacterUpdate(__instance: object): void
    local position = __instance.transform.position
    Position.X = position.x
    Position.Y = position.y
end

return {
    --     Target method,      prehook,   posthook
    {'Centeryidong:FixedUpdate', nil, CharacterUpdate},
}