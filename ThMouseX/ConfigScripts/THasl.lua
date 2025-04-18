-- Runtime = Unity Mono
-- Script to get player's position in Abyss Soul Lotus

local VK_T = 0x54;
local VK_Y = 0x59;

local count = 59;
local x = nil
local y = nil

local function CharacterUpdate(__instance: object): void
    local position = __instance.transform.position
    Position.X = position.x
    Position.Y = position.y
    
    position.z = 99999

    if x ~= nil then position.x = x end
    if y ~= nil then position.y = y end

    count = count + 1
    if count == 60 then
        count = 0        
        if IsKeyDown(VK_T) == true then
            -- x = GetNumberInput();
            -- position.x = x
            print('VK_T')
        end
        if IsKeyDown(VK_Y) == true then
            -- y = GetNumberInput();
            -- position.y = y
            print('VK_Y')
        end
        print("X = " .. position.x, "Y = " .. position.y)
    end
end

return {
    --     Target method,      prehook,   posthook
    {'Centeryidong:FixedUpdate', nil, CharacterUpdate},
}