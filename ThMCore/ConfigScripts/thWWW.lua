-- Script to search player's position in Wonderful Waking World 
-- This game was made in GameMaker Studio
local Obj_Player = 232

ffi.cdef [[
    typedef int* (*getObjectFromId)(int id);
]]

local getObjectFromId = ffi.cast("getObjectFromId", ResolveAddress(AllocNew('int[1]', {0x00077EF0}), 1))

-- native code will call into this function for each frame
function getPositionAddress()
    local object = getObjectFromId(Obj_Player)
    if (object == nil)
        return 0
    objInner = ffi.cast("int*", object[17])
    if (objInner == nil or objInner[2] == 0)
        return 0
    instance = ffi.cast("float*", objInner[2])
    return instance + 44;
end
