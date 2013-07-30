require "strict"

local script        = require "script"
local drawable      = require "graphics.drawable"
local context       = require "graphics.context"
local texture       = require "graphics.texture"
local material      = require "graphics.material"
local camera        = require "graphics.camera"
local positionable  = require "common.positionable"
local world         = require "common.world"
local matrix        = require "math.matrix"
local vector3       = require "math.vector3"
local drawnmesh     = require "graphics.mesh"
local event         = require "common.event"
local input         = require "input.input"
local quaternion    = require "math.quaternion"
local light         = require "graphics.light"
local stage         = require "graphics.stage"
local outpass       = require "graphics.outpass"
local geometrypass  = require "graphics.geometrypass"
local guipass       = require "graphics.guipass"
local frame         = require "graphics.gui.frame"
local image         = require "asset.image"
local text          = require "graphics.gui.text"
local mesh          = require "asset.mesh"
local lightpass     = require "graphics.lightpass"
local file          = require "asset.file"
local heightmap     = require "graphics.heightmap"

local w = world()
local c = context()
c:resize(800, 600, "IntenseLogic Demo")
c.world = w
w.context = c
-- geometry pass
local s = stage()
s.context = c
geometrypass(s)
c:addStage(s, -1)
-- light pass
s = lightpass(c)
c:addStage(s, -1)
-- gui pass
s = guipass(c)
local root = frame()
s:setRoot(root)
c:addStage(s, -1)
-- output pass
c:addStage(outpass(c), -1)

c:setActive()

--[[local marble = texture()
marble:setContext(c)
marble:fromfile("color0", "white-marble-texture.png")

local vf, ff = io.open("shaders/test.vert", "r"), io.open("shaders/test.frag", "r");
local mtl = material()
mtl:vertex(vf:read "*a")
mtl:fragment(ff:read "*a")
vf:close()
ff:close()
mtl:mtlname "Test material"
mtl:arrayAttrib("position", "in_Position")
mtl:arrayAttrib("texcoord", "in_Texcoord")
mtl:arrayAttrib("normal", "in_Normal")
mtl:arrayAttrib("diffuse", "in_Diffuse")
mtl:arrayAttrib("specular", "in_Specular")
mtl:matrix("MVP", "mvp")
mtl:matrix("IMT", "imt")
mtl:textureUnit("color0", "tex")
mtl:fragData("normal", "out_Normal")
mtl:fragData("accumulation", "out_Ambient")
mtl:fragData("diffuse", "out_Diffuse")
mtl:fragData("specular", "out_Specular")
mtl:link(c)
local m = drawnmesh "teapot.obj"
local width = 3
for i = 0, width*width*width-1 do
    local box = positionable()
    w:add(box)
    box.drawable = m --drawable.box
    box.material = mtl
    box.texture = marble
    box.position = (vector3(i % width, math.floor((i%(width*width)) / width), math.floor(i/(width*width))) * vector3(15, 15, 15)).ptr
    box:track(c)
    --print(box.position)
end]]

local ht = texture()
ht:setContext(c)
local hmt = image.loadfile "smooth-heightmap.png"
ht:fromimage("height0", hmt)
ht:fromimage("normal0", hmt:height_to_normal())
local hm = positionable()
w:add(hm)
hm.drawable = heightmap(c, 100, 100)
hm.material = heightmap.defaultShader(c)
hm.texture = ht
hm.position = vector3(0, 0, 0).ptr
hm.size = vector3(100, 25, 100).ptr
hm:track(c)

c.camera = camera()
c.camera.projection_matrix = matrix.perspective(75, 4/3, 2, 1000).ptr
c.camera.positionable.position = vector3(0, 0, 0).ptr
c.camera.sensitivity = .01
c.camera.movespeed = vector3(1,1,1).ptr

lights = {
    {vector3(50, 50, 50),   250,    vector3(.1, .1, .1)},
    {vector3(40, 5, 55),    25,     vector3(.3, .4, 1)},
    {vector3(70, 20, 40),   10,     vector3(1, .5, .2)},
    {vector3(20, 5, 60),    20,     vector3(.8, .7, .1)},
}

for _, t in pairs(lights) do
    local l = light()
    l.positionable.position = t[1].ptr
    l.radius = t[2]
    l.color = t[3].ptr
    l:add(c)
end

--[[local plain = material()
plain:vertex(io.open("shaders/plain.vert","r"):read "*a")
plain:fragment(io.open("shaders/plain.frag", "r"):read "*a")
plain:mtlname "Plain material"
plain:arrayAttrib("position", "in_Position")
plain:matrix("MVP", "mvp")
plain:link(c)
m = drawable.icosahedron(c)
for i = 0, width*width*width - 1 do
    local box = positionable()
    w:add(box)
    box.drawable = m
    box.material = plain
    box.texture = marble
    box.position = (vector3((i % width) + 5, math.floor((i%(width*width)) / width), math.floor(i/(width*width))) * vector3(15, 15, 15)).ptr
    box:track(c)
end]]

local first_mouse = true
function mousemove(reg, name, xabs, yabs, x, y)
    if first_mouse then first_mouse = false return end
    if not input.get "mouse left" then return end
    local yaw = quaternion(vector3(0, 1, 0), x * c.camera.sensitivity)
    local pitch = quaternion(vector3(1, 0, 0), y * c.camera.sensitivity)
    local rot = quaternion.wrap(c.camera.positionable.rotation) * yaw * pitch
    c.camera.positionable.rotation = rot.ptr
end

local georgia = file.load "georgia.ttf"
local camera_pos_label = frame()
camera_pos_label.context = c
camera_pos_label:setPosition(5,5)
root:addChild(camera_pos_label)
function render_pos(pos)
    local label = text(c, "en", "ltr", "latin", georgia, 14, tostring(pos))
    camera_pos_label:setSize(label:getSize())
    camera_pos_label:label(label, {1,1,1,1}, "left middle")
end

function tick(reg, name)
    local get = function(k)
        local b, _ = input.get(k)
        return b and 1 or 0
    end
    local x = get("A") - get("D")
    local z = get("S") - get("W")
    local y = get("F") - get("R")
    local r = get("Q") - get("E")
    local v = vector3(x,y,z) * vector3.wrap(c.camera.movespeed)
    v = v * quaternion.wrap(c.camera.positionable.rotation)
    c.camera.positionable.position = (vector3.wrap(c.camera.positionable.position) + v).ptr
    local bank = quaternion(vector3(0, 0, 1), r * c.camera.sensitivity)
    c.camera.positionable.rotation = (quaternion.wrap(c.camera.positionable.rotation) * bank).ptr
    render_pos(vector3(c.camera.positionable.position))
end

function close(reg, name)
    event.event(event.registry, "shutdown")
end

event.register(event.registry, "tick", tick)
event.register(event.registry, "input.mousemove", mousemove)
event.register(event.registry, "graphics.close", close)

