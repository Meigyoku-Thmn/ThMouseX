-- Script to search player's position in Danmaka: Red Forbidden Fruit
-- This game was made in GameMaker Studio
local ffi = require("ffi")
ffi.cdef [[
    void     OpenConsole     ();
    uint32_t resolveAddress  (uint32_t* offsets, int length);
    uint32_t readUInt32      (uint32_t address);
]]
local ThMouseX = ffi.load('Common.dll')

-- ThMouseX.OpenConsole()

local PLAYER_ELDY = 278
local PLAYER_DIANA = 279
local PLAYER_ANGEL = 280
local HITMARK = 281
local spriteIds = {PLAYER_ELDY, PLAYER_DIANA, PLAYER_ANGEL, HITMARK}
spriteIds[0] = #spriteIds

local baseOffset = 0x023D561C
local objectPoolCountAddress = ThMouseX.resolveAddress(ffi.new('uint32_t[1]', {baseOffset + 2 * 4}), 1)

local positionOffset = 0xB4
local spriteIdOffset = 0x7C

local posAddressChain = ffi.new('uint32_t[3]', {baseOffset, 0, positionOffset})
local spriteIdAddressChain = ffi.new('uint32_t[4]', {baseOffset, 0, spriteIdOffset, 0})

local objectFound = false

local function testAndGetAddress(objectIdx)
    if (objectIdx) then
        posAddressChain[1] = objectIdx * 4
        spriteIdAddressChain[1] = objectIdx * 4
    end
    local spriteId = ThMouseX.resolveAddress(spriteIdAddressChain, 4)
    if (spriteId == 0) then
        return 0
    end
    for i = 1, spriteIds[0] do
        if (spriteId == spriteIds[i]) then
            return ThMouseX.resolveAddress(posAddressChain, 3)
        end
    end
    return 0
end

-- native code will call into this function for each frame
function getPositionAddress()
    -- use cache address
    if (objectFound == true) then
        local address = testAndGetAddress()
        if (address ~= 0) then
            return address
        end
    end
    -- scan address range if presets scan fails, this is for practice mode
    for objectIdx = 0, ThMouseX.readUInt32(objectPoolCountAddress) - 1 do
        local address = testAndGetAddress(objectIdx)
        if (address ~= 0) then
            objectFound = true
            return address
        end
    end
    -- remove cache if all fail
    objectFound = false
    return 0
end
