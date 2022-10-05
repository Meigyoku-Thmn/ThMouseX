-- Script to search player's position in Danmaka: Red Forbidden Fruit
-- This game was made in GameMaker Studio
local ffi = require("ffi")
ffi.cdef [[
    uint32_t resolveAddress  (uint32_t* offsets, int length);
    bool     compareToString (uint32_t address, const char* str);
]]
local ThMouseX = ffi.load('Common.dll')

local names = {'hitmark', 'player_eldy', 'player_diana', 'player_ange'}

local baseOffset = 0x023D561C

-- start from incompleted sprite addresses
local positionAddressChains = { --
    {baseOffset, 0x1B4}, -- Stage 1
    {baseOffset, 0x044}, -- Stage 2
    {baseOffset, 0x0C4}, -- Stage 3
    {baseOffset, 0x02C}, -- Stage 4, Stage 5, Stage 6
    {baseOffset, 0x060}, -- Stage Extra
    {baseOffset, 0x030} --- Challenges
}
local positionOffsets = {0xB4}
local positionOffsetNumber = #positionAddressChains[1] + #positionOffsets

local nameOffsets = {0x80, 0x14, 0x00}
local nameOffsetNumber = #positionAddressChains[1] + #nameOffsets
local nameAddressChains = {}

-- build sprite position addresses and sprite name addresses
for i = 1, #positionAddressChains do
    table.insert(nameAddressChains, {unpack(positionAddressChains[i])})

    for _, positionOffset in ipairs(positionOffsets) do
        table.insert(positionAddressChains[i], positionOffset)
    end
    positionAddressChains[i] = ffi.new('uint32_t[' .. #positionAddressChains[i] .. ']', positionAddressChains[i])

    for _, nameOffset in ipairs(nameOffsets) do
        table.insert(nameAddressChains[i], nameOffset)
    end
    nameAddressChains[i] = ffi.new('uint32_t[' .. #nameAddressChains[i] .. ']', nameAddressChains[i])
end

-- build sprite position addresses and sprite name addresses for case 3: scan address range
local unk_positionAddressChains = {}
local unk_nameAddressChains = {}
-- [200, 400] should be enough for this game
for idx = 200, 400 do
    local offset = idx * 4

    local posAddrChain = {baseOffset, offset, unpack(positionOffsets)}
    table.insert(unk_positionAddressChains, ffi.new('uint32_t[' .. #posAddrChain .. ']', posAddrChain))

    local nameAddrChain = {baseOffset, offset, unpack(nameOffsets)}
    table.insert(unk_nameAddressChains, ffi.new('uint32_t[' .. #nameAddrChain .. ']', nameAddrChain))
end

-- for-loop optimizations
positionAddressChains[0] = #positionAddressChains
nameAddressChains[0] = #nameAddressChains
unk_positionAddressChains[0] = #unk_positionAddressChains
unk_nameAddressChains[0] = #unk_nameAddressChains
names[0] = #names

local function resolveAddressAndCompareToStrings(posAddressChain, nameAddressChain)
    local nameAddress = ThMouseX.resolveAddress(nameAddressChain, nameOffsetNumber)
    if (nameAddress == 0) then
        return 0
    end
    for i = 1, names[0] do
        if (ThMouseX.compareToString(nameAddress, names[i]) == true) then
            return ThMouseX.resolveAddress(posAddressChain, positionOffsetNumber)
        end
    end
    return 0
end

-- cache
local lastFoundPositionAddressChain = nil
local lastFoundNameAddressChain = nil

-- native code will call into this function for each frame
function getPositionAddress()
    -- use cache address
    if lastFoundPositionAddressChain then
        local address = resolveAddressAndCompareToStrings(lastFoundPositionAddressChain, lastFoundNameAddressChain)
        if (address ~= 0) then
            return address
        end
    end
    -- use preset address if cache misses
    for i = 1, positionAddressChains[0] do
        local address = resolveAddressAndCompareToStrings(positionAddressChains[i], nameAddressChains[i])
        if (address ~= 0) then
            lastFoundPositionAddressChain = positionAddressChains[i]
            lastFoundNameAddressChain = nameAddressChains[i]
            return address
        end
    end
    -- scan address range if presets scan fails, this is for practice mode
    for i = 1, unk_positionAddressChains[0] do
        local address = resolveAddressAndCompareToStrings(unk_positionAddressChains[i], unk_nameAddressChains[i])
        if (address ~= 0) then
            lastFoundPositionAddressChain = unk_positionAddressChains[i]
            lastFoundNameAddressChain = unk_nameAddressChains[i]
            return address
        end
    end
    -- remove cache if all fail
    lastFoundPositionAddressChain = nil
    lastFoundNameAddressChain = nil
    return 0
end
