#include "event.h"

#include <stdlib.h>
#include <string.h>

#include "common/log.h"

void il_Event_push(const il_Event_Event* event) {
  struct il_Event_Node *node = malloc(sizeof(struct il_Event_Node));
  
  node->event = (il_Event_Event*)event;
  node->next = NULL;
  
  if (il_Event_EventQueue_last != NULL)
    il_Event_EventQueue_last->next = node;
  il_Event_EventQueue_last = node;
  if (il_Event_EventQueue_first == NULL)
    il_Event_EventQueue_first = node;
}

void il_Event_pushnew(unsigned id, size_t size, void * data) {
  il_Event_Event *ev = malloc(sizeof(il_Event_Event) + size);
  if (id > 65535) il_Common_log(1, "Event ID is out of bounds: %u\n", id);
  ev->eventid = (uint8_t)id;
  if (size > 255) il_Common_log(1, "Size of event is out of bounds: %u\n", size);
  ev->size = (uint16_t)size;
  if (data != NULL) {
    memcpy(&ev->data, data, size);
  }
  il_Event_push(ev);
}

const il_Event_Event* il_Event_pop() {
  struct il_Event_Node *node = il_Event_EventQueue_first;
  if (node == NULL) {
    return NULL;
  }
  if (node == il_Event_EventQueue_last) {
    il_Event_EventQueue_last = NULL;
  }
  il_Event_EventQueue_first = node->next;
  
  il_Event_Event* ev = node->event;
  free(node);
  return ev;
}

void il_Event_handle(il_Event_Event* ev) {
  int i;
  struct il_Event_CallbackContainer* container = NULL;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == ev->eventid) {
      container = &il_Event_Callbacks[i];
      break;
    }
  }
  
  if (!container || !container->callbacks) {
    return;
  }
  
  for (i = 0; i < container->length; i++) {
    container->callbacks[i](ev);
  }
  
  free(ev);
  
}

void il_Event_register(uint16_t eventid, il_Event_Callback callback) {
  int i;
  struct il_Event_CallbackContainer* container = NULL;
  for (i = 0; i < il_Event_Callbacks_len; i++) {
    if (il_Event_Callbacks[i].eventid == eventid) {
      container = &il_Event_Callbacks[i];
      break;
    }
  }
  
  if (container == NULL) {
    container = malloc(sizeof(struct il_Event_CallbackContainer));
    container->eventid = eventid;
    container->length = 0;
    container->callbacks = NULL;
  }
  
  il_Event_Callback* temp = (il_Event_Callback*)malloc(sizeof(il_Event_Callback) * (container->length+1));
  memcpy(temp, container->callbacks, sizeof(il_Event_Callback) * container->length);
  //if (container->callbacks != NULL)
  free(container->callbacks);
  temp[container->length] = callback;
  container->length++;
  container->callbacks = temp;
  
  il_Event_CallbackContainer *temp2 = (il_Event_CallbackContainer*)malloc(sizeof(il_Event_CallbackContainer) * (il_Event_Callbacks_len+1));
  memcpy(temp2, il_Event_Callbacks, sizeof(il_Event_CallbackContainer) * il_Event_Callbacks_len);
  temp2[il_Event_Callbacks_len] = *container;
  free(il_Event_Callbacks);
  il_Event_Callbacks = temp2;
  il_Event_Callbacks_len++;
  
}
