#include "frame.h"

#include "util/log.h"
#include "graphics/context.h"

il_type ilG_gui_frame_type = {
    .typeclasses = NULL,
    .storage = {NULL},
    .constructor = NULL,
    .destructor = NULL,
    .copy = NULL,
    .name = "il.graphics.gui.frame",
    .size = sizeof(ilG_gui_frame),
    .parent = NULL
};

static void frame_draw(void *ptr)
{
    ilG_gui_frame *self = ptr;
    ilG_gui_draw(self);
}

static int frame_build(void *ptr, struct ilG_context *context)
{
    ilG_gui_frame *self = ptr;
    self->context = context;
    self->complete = self->build? self->build(self, context) : 1;
    for (unsigned i = 0; i < self->children.length; i++) {
        frame_build(self->children.data[i], context);
    }
    return self->complete;
}

static il_table *frame_get_storage(void *ptr)
{
    ilG_gui_frame *self = ptr;
    return &self->base.storage;
}

static bool frame_get_complete(const void *ptr)
{
    const ilG_gui_frame *self = ptr;
    return self->complete;
}

static void frame_add_renderer(ilG_gui_frame *self, ilG_renderer r)
{
    if (r.vtable != &ilG_gui_frame_renderer) {
        il_error("Cannot add non-frame to frame");
        return;
    }
    IL_APPEND(self->children, r.obj);
}

static void frame_del_renderer(ilG_gui_frame *self, ilG_renderer node)
{
    for (unsigned i = 0; i < self->children.length; i++) {
        if (self->children.data[i] == node.obj) {
            IL_REMOVE(self->children, i);
            break;
        }
    }
}

static void frame_message(void *ptr, int type, il_value *v)
{
    ilG_gui_frame *self = ptr;
    switch (type) {
    case 1:
        frame_add_renderer(self, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    case 2:
        frame_del_renderer(self, *(ilG_renderer*)il_value_tomvoid(v));
        break;
    default:
        self->message(ptr, type, v);
    }
}

static void frame_push_msg(void *ptr, int type, il_value v)
{
    ilG_gui_frame *self = ptr;
    if (self->context) {
        ilG_context_message(self->context, ilG_gui_frame_wrap(self), type, v);
    } else {
        frame_message(ptr, type, &v);
        il_value_free(v);
    }
}

const ilG_renderable ilG_gui_frame_renderer = {
    .free = il_unref,
    .draw = frame_draw,
    .build = frame_build,
    .get_storage = frame_get_storage,
    .get_complete = frame_get_complete,
    .add_renderer = 1,
    .del_renderer = 2,
    .message = frame_message,
    .push_msg = frame_push_msg,
    .name = "GUI Frame"
};

static ilG_gui_coord computecoordabs(ilG_gui_coord coord, ilG_gui_rect parent)
{
    ilG_gui_coord size, res = {0,0,0,0};
    size.x = parent.b.x - parent.a.x;
    size.y = parent.b.y - parent.a.y;
    res.x = coord.x + size.x * coord.xp + parent.a.x;
    res.y = coord.y + size.y * coord.yp + parent.a.y;
    return res;
}

static ilG_gui_rect computerectabs(ilG_gui_rect rect, ilG_gui_rect parent)
{
    ilG_gui_rect res;
    res.a = computecoordabs(rect.a, parent);
    res.b = computecoordabs(rect.b, parent);
    return res;
}

ilG_gui_rect ilG_gui_frame_abs(ilG_gui_frame *self)
{
    if (self->parent) {
        return computerectabs(self->rect, ilG_gui_frame_abs(self->parent));
    } else {
        // all we can do is ignore the percentage field and treat the size as absolute
        ilG_gui_rect rect = {{0,0,0,0},{0,0,0,0}};
        rect.a.x = self->rect.a.x;
        rect.a.y = self->rect.a.y;
        rect.b.x = self->rect.b.x;
        rect.b.y = self->rect.b.y;
        return rect;
    }
}

int ilG_gui_frame_contains(ilG_gui_frame *self, ilG_gui_coord coord)
{
    ilG_gui_rect rect = ilG_gui_frame_abs(self);
    /*ilG_gui_coord size;
    size.x = rect.b.x - rect.a.x;
    size.y = rect.b.y - rect.a.y;*/
    coord = computecoordabs(coord, rect);
    return coord.x >= rect.a.x && coord.y >= rect.a.y &&
           coord.x <  rect.b.x && coord.y <  rect.b.y;
}

enum ilG_gui_inputaction ilG_gui_click(ilG_gui_frame *top, int x, int y, int button)
{
    ilG_gui_coord mouse = {x,y, 0.f, 0.f};
    if (!ilG_gui_frame_contains(top, mouse)) {
        return ILG_GUI_UNHANDLED;
    }
    enum ilG_gui_inputaction res = ILG_GUI_UNHANDLED;
    if (top->click) {
        res = top->click(top, x, y, button);
    }
    if (res == ILG_GUI_OVERRIDE) {
        return res;
    }
    ilG_gui_frame *cur = top;
    size_t i;
    for (i = 0; i < cur->children.length; i++) {
        enum ilG_gui_inputaction res2 = ilG_gui_click(cur->children.data[i], x, y, button);
        if (res2 > res) {
            res = res2;
        }
        if (res == ILG_GUI_OVERRIDE) {
            break;
        }
    }
    return res;
}

void ilG_gui_hover(ilG_gui_frame *top, int x, int y)
{
    (void)top, (void)x, (void)y;
}

static void draw(ilG_gui_frame *top, ilG_gui_rect rect)
{
    if (top->draw) {
        top->draw(top, rect);
    }
    size_t i;
    for (i = 0; i < top->children.length; i++) {
        draw(top->children.data[i], computerectabs(top->children.data[i]->rect, rect));
    }
}

void ilG_gui_draw(ilG_gui_frame *top)
{
    ilG_gui_rect rect = ilG_gui_frame_abs(top);
    draw(top, rect);
}

void ilG_gui_addChild(ilG_gui_frame *parent, ilG_gui_frame *child)
{
    IL_APPEND(parent->children, child);
    child->parent = parent;
}

void ilG_gui_pop(ilG_gui_frame *node)
{
    ilG_gui_frame *parent = node->parent;
    unsigned i;
    for (i = 0; i < parent->children.length; i++) {
        if (parent->children.data[i] == node) {
            parent->children.data[i] = parent->children.data[--parent->children.length];
            break;
        }
    }
    node->parent = NULL;
}

