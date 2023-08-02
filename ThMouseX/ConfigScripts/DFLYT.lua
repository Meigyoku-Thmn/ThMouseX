-- LuaDllName = lua.dll
-- Script to get player's position in Chaos of Black Loong
-- This game was made in LuaSTGPlus

InitializeForLuaJIT();
local ffi = require("ffi")
local position = ffi.new('double[2]', {0, 0})

-- native code will call into this function for each frame
function getPositionAddress()
    local _player = rawget(_G, "player")
    position[0] = (_player and _player.x or 0) + 224
    position[1] = 480 - ((_player and _player.y or 0) + 239) - 1 -- y axis is inverted
    return tonumber(ffi.cast("DWORD", position))
end