-- LuaDllName = lua.dll
-- Script to get player's position in Chaos of Black Loong
-- This game was made in LuaSTGPlus

local ori_metatb = getmetatable(_G)
InitializeForLuaJIT();

local function Setup()
    local ffi = require("ffi")

    local position = ffi.new('double[2]', {})
    SetPositionAddress(ffi.cast("DWORD", position))

    local ori_FrameFunc = FrameFunc
    FrameFunc = function()
        local rs = ori_FrameFunc()
        position[0] = player and player.x or 0
        position[1] = player and player.y or 0
        return rs
    end

    local onCloseCb = ffi.cast("UninitializeCallbackType", function(isProcessTerminating)
        FrameFunc = ori_FrameFunc
        onCloseCb:free()
    end)
    RegisterUninitializeCallback(onCloseCb)
end

if (FrameFunc ~= nil) then
    Setup()
else
    local unpack = unpack or table.unpack
    local metatb = {unpack (ori_metatb or {})}
    function metatb:__newindex(name, value)
        rawset(self, name, value)
        if (name == "FrameFunc") then
            setmetatable(_G, ori_metatb)
            Setup()
        end
    end
    setmetatable(_G, metatb)
    local onCloseCb = ffi.cast("UninitializeCallbackType", function(isProcessTerminating)
        setmetatable(_G, ori_metatb)
        onCloseCb:free()
    end)
    RegisterUninitializeCallback(onCloseCb)
end