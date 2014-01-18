/** @file loader.h
 * @brief Responsible for loading modules.
 */

#ifndef IL_LOADER_H
#define IL_LOADER_H

#ifdef WIN32
#   include <windows.h>
    typedef FARPROC WINAPI il_func;
#else
    /** A generic function pointer. */
    typedef void *il_func;
#endif

/** A pointer to the symbol used to bootstrap a module.
 * @return Whether the module was bootstrapped correctly. Zero indicates
 * success.
 */
typedef int (*il_bootstrap_fn)(int argc, char **argv);

/** A pointer to the symbol used to indicate dependencies of a module.
 * @return An array of dependencies.
 */
typedef const char** (*il_dependencies_fn)(int argc, char **argv);

/** Sets a certain plugin to not load
 * @param name The name of the plugin
 */
void il_ignore_module(const char *name);

/** Adds a path to be searched when loading modules.
 * @param path The path to add to the list of module paths.
 */
void il_add_module_path(const char *path);

/** Loads a module by name.
 * @param module The name of the module to load.
 * @param argc The argument count passed to main.
 * @param argv The arguments passed to main.
 * @return Whether the module was loaded. Zero indicates success.
 */
int il_load_module(const char *module, int argc, char **argv);

/** Loads all the modules in a directory.
 * @param path The path of the directory to load.
 * @param argc The argument count passed to main.
 * @param argv The arguments passed to main.
 */
void il_load_module_dir(const char *path, int argc, char **argv);

/** Loads all the modules in the paths added by il_add_module_path().
 * @param argc The argument count passed to main.
 * @param argv The arguments passed to main.
 */
void il_load_module_paths(int argc, char **argv);

/** Closes a loaded module.
 * @param module The name of the module to close.
 */
void il_close_module(const char *module);

/** Gets a symbol defined in a module.
 * @param module The name of the module to search in.
 * @param name The name of the symbol to get.
 * @return The address of the symbol.
 */
#ifdef IL_DLSYM_LOADER
# ifdef WIN32
static il_func il_get_symbol(const char *module, const char *name)
{
    static il_func (*sym)(const char*, const char*);
    static HMODULE handle;
    if (!sym) {
        handle = GetModuleHandle(NULL); // get current process's exe file
        sym = (il_func(*)(const char*, const char*))GetProcAddress(handle, "il_get_symbol");
    }
    return sym(module, name);
}
# else
#  include <dlfcn.h>
static il_func il_get_symbol(const char *module, const char *name)
{
    static il_func (*sym)(const char*,const char*);
    if (!sym) {
        sym = (il_func(*)(const char*,const char*))dlsym(0, "il_get_symbol");
    }
    return sym(module, name);
}
# endif
#else
il_func il_get_symbol(const char *module, const char *name);
#endif

#define il_gen_arglist(a,b) a b 
#define il_gen_calllist(a,b) b 

#define il_gen_noret_symbol(mod, name, args) \
    static void name(args(il_gen_arglist)) \
    { \
        static il_func sym;                     \
        static int have_sym;                    \
        if (!have_sym) {                        \
            sym = il_get_symbol(mod, #name);    \
            have_sym = 1;                       \
        }                                       \
        if (sym) {                              \
            ((void (*)(args(il_gen_arglist)))sym)(args(il_gen_calllist)); \
        }                                       \
    }

#define il_gen_symbol(mod, def, ret, name, args) \
    static ret name(args(il_gen_arglist)) \
    {                                           \
        static il_func sym;                     \
        static int have_sym;                    \
        if (!have_sym) {                        \
            sym = il_get_symbol(mod, #name);    \
            have_sym = 1;                       \
        }                                       \
        if (sym) {                              \
            return ((ret (*)(args(il_gen_arglist)))sym)(args(il_gen_calllist)); \
        }                                       \
        return def;                             \
    }

/** Iterates through a module. Call until it returns NULL.
 * @param saveptr A pointer to store the iterator in. Initialize to NULL.
 * @return The next module.
 */
const char *il_module_iterate(void **saveptr);

#endif

