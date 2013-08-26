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
local skyboxpass    = require "graphics.skyboxpass"

math.randomseed(os.time())

local w = world()
local c = context()
c:resize(800, 600, "IntenseLogic Demo")
c.world = w
w.context = c
-- skybox pass
local skybox = texture()
skybox:setContext(c)
test_img = image.loadfile "demos/bouncing-lights/stars.png"
skybox:cubemap("color0", {test_img, test_img, test_img, test_img, test_img, test_img})
local s = stage()
s.context = c
skyboxpass(s, skybox)
c:addStage(s, -1)
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

local ht = texture()
ht:setContext(c)
local hmt = image.loadfile "demos/bouncing-lights/smooth-heightmap.png"
ht:fromimage("height0", hmt)
ht:fromimage("normal0", hmt:height_to_normal())
ht:fromfile("color0", "white-marble-texture.png")
local hm = positionable()
w:add(hm)
hm.drawable = heightmap(c, 100, 100)
hm.material = heightmap.defaultShader(c)
hm.texture = ht
hm.position = vector3(0, 0, 0).ptr
hm.size = vector3(100, 50, 100).ptr
hm:track(c)

c.camera = camera()
c.camera.projection_matrix = matrix.perspective(75, 4/3, 2, 2000).ptr
c.camera.positionable.position = vector3(0, 0, 0).ptr
c.camera.sensitivity = .01
c.camera.movespeed = vector3(1,1,1).ptr

local hw, hh = 100, 100
for i = 1, 100 do
    local l = light()
    local pos = vector3(math.random(0,hw-1), 0, math.random(0,hh-1))
    local height = hmt:getPixel(pos.x, pos.z) 
    pos.y = height * 50 + 2
    l.positionable.position = pos.ptr
    l.radius = math.random(1, 15)
    l.color = vector3(math.random(0,1), math.random(0,1), math.random(0,1)).ptr
    l:add(c)
end

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
camera_pos_label:setPosition(5,-19, 0, 1)
root:addChild(camera_pos_label)
function render_pos(pos)
    local label = text(c, "en", "ltr", "latin", georgia, 14, tostring(pos))
    camera_pos_label:label(label, {1,1,1,1}, "left middle")
end

local fps_label = frame()
fps_label.context = c
fps_label:setPosition(5, 5, 0, 0)
root:addChild(fps_label)
function render_fps(f)
    local label = text(c, "en", "ltr", "latin", georgia, 14, string.format("FPS: %.1f", tonumber(f)))
    fps_label:label(label, {1,1,1,1}, "left middle")
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
    local bank = quaternion(vector3(0, 0, 1), r * c.camera.sensitivity * 4)
    c.camera.positionable.rotation = (quaternion.wrap(c.camera.positionable.rotation) * bank).ptr
    render_pos(vector3(c.camera.positionable.position))
    render_fps(1/c:averageFrametime())
    c.camera.projection_matrix = matrix.perspective(75, c.width/c.height, 2, 2000).ptr
end

function close(reg, name)
    event.event(event.registry, "shutdown")
end

event.register(event.registry, "tick", tick)
event.register(event.registry, "input.mousemove", mousemove)
event.register(event.registry, "graphics.close", close)

