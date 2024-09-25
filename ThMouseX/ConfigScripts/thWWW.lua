-- Script to get player's position in Wonderful Waking World 
-- This game was made in GameMaker Studio
local ffi = require("ffi")

ffi.cdef [[
    typedef int* (*getObjectFromId)(int id);
]]

local Obj_Player_Id = 232
local getObjectFromId = ffi.cast("getObjectFromId", ResolveAddress(ffi.new('uint32_t[1]', {0x34A30}), 1, false))

-- native code will call into this function for each frame
function getPositionAddress()
    local object = getObjectFromId(Obj_Player_Id)
    if (object == nil) then
        return 0
    end
    objInner = ffi.cast("int*", object[17])
    if (objInner == nil or objInner[2] == 0) then
        return 0
    end
    instance = ffi.cast("int", objInner[2])
    return tonumber(instance + 44 * 4)
end