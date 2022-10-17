local function CharacterUpdate(___body: object): void
    local position = ___body.position
    Pos.X = position.X
    Pos.Y = position.Y
end

return {
    --    Target method,     prehook,   posthook
    {'THMHJ.Character:Update', nil, CharacterUpdate},
}