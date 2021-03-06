/** @file base.h
 * @brief Object system for IL
 */

#ifndef IL_BASE_H
#define IL_BASE_H

#include <stdlib.h>

#include "util/array.h"
#include "util/uthash.h"
#include "common/event.h"
#include "common/storage.h"

/** Header that should be at the top of every typeclass */
#define il_typeclass_header const char *name; UT_hash_handle hh

/** Null typeclass */
typedef struct il_typeclass {
    il_typeclass_header;
} il_typeclass;

typedef struct il_base il_base;
typedef struct il_type il_type;

typedef void (*il_base_init_fn)(void *base);
typedef il_base *(*il_base_copy_fn)(void *base, const void *base2);
typedef void (*il_base_free_fn)(void *base);

/** Kind of like a class */
struct il_type {
    il_typeclass *typeclasses;      /** Hash table of trait implementations */
    il_table storage;               /** Storage for per-type data */
    il_base_init_fn constructor;    /** Called when il_new() and il_init() are called */
    il_base_free_fn destructor;     /** Called when il_unref() sees a zero-count object */
    il_base_copy_fn copy;           /** Called when il_copy() is invoked */
    const char *name;               /** The name of the type */
    size_t size;                    /** Size of the structure this type object represents */
    il_type *parent;                /** Parent type */
};

/** Header for instances of il_types */
struct il_base {
    int refs;                       /** Reference counter */
    il_base_free_fn free;           /** Function to free memory after object is no longer needed */
    il_table storage;               /** Key-value storage */
    il_base *gc_next;               /** Unused */
    IL_ARRAY(il_base**,) weak_refs; /** Weak references, see il_weakref() */
    il_type *type;                  /** Type object for this instance */
};

void *il_ref(void *obj);
void il_unref(void* obj);
/** Creates a weak reference. This will be set to NULL when the object is freed */
void il_weakref(void *obj, void **ptr);
void il_weakunref(void *obj, void **ptr);
il_table *il_base_getStorage(void *obj);
il_table *il_type_getStorage(il_type *T);
size_t il_sizeof(const il_type *self);
/** Returns the type object for any il_base */
il_type *il_typeof(void *obj);
/** Allocates a new object */
void *il_new(il_type *type);
/** Initializes a sufficiently sized region of memory to a new object */
void il_init(il_type *type, void *obj);
/** Creates a deep copy of an object */
void *il_copy(void *obj);
/** Returns a pointer to the name of a type, do not free it */
const char *il_name(il_type *type);
/** Looks up the typeclass implementation for the name to */
const void *il_cast(il_type* T, const char *to);
/** Registers an implementation for a typeclass */
void il_impl(il_type* T, void *impl);

#endif

