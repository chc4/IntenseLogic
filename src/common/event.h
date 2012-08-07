#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <stdint.h>
#include <stddef.h>

typedef struct il_Event_Event {
  uint16_t eventid;
  uint8_t size;
  uint8_t data[];
} il_Event_Event;

typedef void(*il_Event_Callback)(const il_Event_Event*);

void il_Event_push(const il_Event_Event* event);

void il_Event_pushnew(unsigned id, size_t size, const void * data);

const il_Event_Event* il_Event_pop();

void il_Event_handle(il_Event_Event* ev);

void il_Event_register(uint16_t eventid, il_Event_Callback callback);

#endif
