-- Runtime = Unity Mono
-- Script to get player's position in Abyss Soul Lotus

local VK_T = 0x54;
local VK_Y = 0x59;

local count = 59;

local function CharacterUpdate(__instance: object): void
    local position = __instance.transform.position
    Position.X = position.x
    Position.Y = position.y
    
    position.z = 99999

    count = count + 1
    if count == 60 then
        count = 0        
        if IsKeyDown(VK_T) == true then
            -- position.x = GetNumberInput();
            print('VK_T')
        end
        if IsKeyDown(VK_Y) == true then
            -- position.y = GetNumberInput();
            print('VK_Y')
        end
        print("X = " .. position.x, "Y = " .. position.y)
    end
end

return {
    --     Target method,      prehook,   posthook
    {'Centeryidong:FixedUpdate', nil, CharacterUpdate},
}