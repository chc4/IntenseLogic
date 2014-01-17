---
-- Provides functions for working with vec3 types
-- @author tiffany
local ffi = require "ffi"

require "math.scalar_defs"
require "math.vector4"

ffi.cdef [[

//#define il_vec3_new(x, y, z) il_vec4_new(x, y, z, 1.0)
char *il_vec3_print(const il_vec3 v, char *buf, unsigned length);

//#define il_vec3_add il_vec4_add
//#define il_vec3_sub il_vec4_sub
//#define il_vec3_mul il_vec4_mul
//#define il_vec3_div il_vec4_div
il_vec3 il_vec3_rotate(const il_vec3 a, const il_quat q);
il_vec3 il_vec3_cross(const il_vec3 a, const il_vec3 b);
float il_vec3_dot(const il_vec3 a, const il_vec3 b);
il_vec3 il_vec3_normal(const il_vec3 a);
il_vec4 il_vec3_to_vec4(const il_vec3 a, float w);
float il_vec3_len(const il_vec3 a);

]]

local quaternion
local vector4

local vector3 = {}

--- FFI ctype
vector3.type = ffi.typeof "il_vec3"

--- Returns whether the parameter is a vec3
-- @tparam vec3 obj
-- @treturn bool
function vector3.check(obj)
    return type(obj) == "table" and obj.T == "vec3" and ffi.istype(vector3.type, obj.ptr)
end

local function c_wrap(f)
    return function(a,b)
        assert(vector3.check(a), "Bad argument #1: Expected vector3, got "..type(a))
        assert(vector3.check(b), "Bad argument #2: Expected vector3, got "..type(b))
        return vector3.wrap(f(a.ptr, b.ptr))
    end
end

local add = c_wrap(modules.math.il_vec4_add)
local sub = c_wrap(modules.math.il_vec4_sub)
local oldmul = c_wrap(modules.math.il_vec4_mul)
local div = c_wrap(modules.math.il_vec4_div)

local function mul(a,b)
    quaternion = quaternion or require "math.quaternion"
    if quaternion.check(b) then
        return vector3.wrap(modules.math.il_vec3_rotate(a.ptr, b.ptr))
    else
        return oldmul(a,b)
    end
end

local axis = {"x", "y", "z"}
local function index(t, k)
    if k == "x" then
        return t.ptr.x
    elseif k == "y" then
        return t.ptr.y
    elseif k == "z" then
        return t.ptr.z
    elseif k == "len" then
        return modules.math.il_vec3_len(t.ptr)
    elseif k == "normal" then
        return vector3.wrap(modules.math.il_vec3_normal(t.ptr))
    elseif k == "vec4" then
        vector4 = vector4 or require "math.vector4"
        return vector4.wrap(modules.math.il_vec3_to_vec4(t.ptr))
    end
    return vector3[axis[k]]
end

local function newindex(t, k, v)
    assert(type(v) == "number")
    if k == "x" then
        t.ptr.x = v
        return
    elseif k == "y" then
        t.ptr.y = v
        return
    elseif k == "z" then
        t.ptr.z = v
        return
    end
    error("Invalid key "..tostring(k).." in vector3")
end

local function ts(t)
    return string.format("(%.6f, %.6f, %.6f)", t.x, t.y, t.z)
end

local function gc(obj)
    modules.math.il_vec4_free(obj.ptr);
end

local function eq(a, b)
    return a.x == b.x and a.y == b.y and a.z == b.z
end

local function lt(a, b)
    return a.x < b.x and a.y < b.y and a.z < b.z
end

--- Wraps the vec3 with a metatable
function vector3.wrap(ptr)
    local obj = {}
    obj.ptr = ptr;
    obj.T = "vec3"
    setmetatable(obj, {__index=index, __newindex=newindex, __tostring=ts, __add=add, __sub=sub, __mul=mul, __div=div, __gc=gc, __eq=eq, __lt=lt})
    return obj;
end

--- Computes the dot product of two vec3s
-- @tparam vec3 a
-- @tparam vec3 b
-- @treturn vec3
function vector3.dot(a,b)
    assert(vector3.check(a) and vector3.check(b))
    return modules.math.il_vec3_dot(a.ptr, b.ptr)
end

--- Computes the cross product of two vec3s
-- @tparam vec3 a
-- @tparam vec3 b
-- @treturn vec3
function vector3.cross(a,b)
    assert(vector3.check(a) and vector3.check(b))
    return vector3.wrap(modules.math.il_vec3_cross(a.ptr, b.ptr))
end

--- Creates a copy of a vec3
-- @tparam vec3 v
-- @treturn vec3
function vector3.copy(v)
    assert(vector3.check(v))
    return vector3.wrap(v.ptr)
end

--- Creates a new vec3
-- @tparam ?number x
-- @tparam ?number y
-- @tparam ?number z
-- @treturn vec3
function vector3.create(x, y, z)
    if not x then
        return vector3.wrap(modules.math.il_vec4_new(0, 0, 0, 1))
    elseif type(x) == "number" and not y then
        return vector3.wrap(modules.math.il_vec4_new(x, x, x, 1.0))
    elseif vector3.check(x) then
        return vector3.wrap(x.ptr)
    elseif ffi.istype(vector3.type, x) then
        return vector3.wrap(x)
    elseif x and y and z then
        assert(type(y) == "number" and
               type(z) == "number")
        return vector3.wrap(modules.math.il_vec4_new(x, y, z, 1.0))
    end
end

setmetatable(vector3, {__call=function(self,...) return vector3.create(...) end})

return vector3

