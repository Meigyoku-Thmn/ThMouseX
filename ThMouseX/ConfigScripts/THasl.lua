-- Runtime = Unity Mono
-- Script to get player's position in Abyss Soul Lotus
local function CharacterUpdate(___transform: object): void
    local position = ___transform.position
    Position.X = position.x
    Position.Y = position.y
end

return {
    --     Target method,      prehook,   posthook
    {'Centeryidong:FixedUpdate', nil, CharacterUpdate},
}