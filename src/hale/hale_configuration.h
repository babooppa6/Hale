#ifndef HALE_CONFIGURATION_H
#define HALE_CONFIGURATION_H

#if HALE_INCLUDES
#include "hale_config.h"
#include "hale_math.h"
#include "hale_key.h"
#include "hale_set.h"
#include "hale_stack_memory.h"
#endif

#if HALE_USE_INITIALIZER_LIST
#include <initializer_list>
#endif


namespace hale {

struct Scope;

struct EventHandlerCallArgs
{
    Scope *scope;

    template<typename T>
    T get(memi index) { return 0; }
};

struct EventHandler;

#define HALE_EVENT_HANDLER(name) void name(HALE_STACK, EventHandler *handler, EventHandlerCallArgs *args)
typedef HALE_EVENT_HANDLER(EventHandlerFunction);

struct EventHandler {
    struct Args
    {
    };

    EventHandlerFunction *function;
    Args *args;
    memi size;
    void *ptr;
};

struct KeyEventHandler {
    Key key;
    EventHandler handler;
};

struct ConfigureEventHandler {
    EventHandler handler;
};

struct ConfigureValue
{
    enum Type : u8
    {
        Type_Null = 0,

        Type_Margin,
        Type_Color,
        Type_String,
        Type_Real32,
        Type_Real64,
        Type_Signed32,
        Type_Unsigned32,
        Type_Signed64,
        Type_Unsigned64,

        Type_KeyEventHandler,
        Type_ConfigureEventHandler,
    };

    Type type;

    union
    {
        sptr Ptr;
        Margin<r32> Margin;
        Color32 Color;
        ch8 *String;
        r32 Real32;
        r64 Real64;
        s32 Signed32;
        u32 Unsigned32;
        s64 Signed64;
        u64 Unsigned64;
        hale::KeyEventHandler KeyEventHandler;
        hale::ConfigureEventHandler ConfigureEventHandler;
    };
};

struct ConfigureProperty
{
    u8 *scope;
    u8 *key;
    ConfigureValue value;
};

//
//
//

struct ScopeSelector
{
    u8 count;
    u8** names;
};

//
//
//

struct ScopeData
{
    KeyEventHandler* key_down;
    memi key_down_count;
    KeyEventHandler* key_up;
    memi key_up_count;
    KeyEventHandler* text;
    memi text_count;
    ConfigureEventHandler *configure;
    memi configure_count;

    // Deprecated
    ConfigureProperty* properties;
    memi properties_count;
};

struct ScopeSelectorData
{
    ScopeSelector selector;
    ScopeData data;
};

#define HALE_SCOPE_MAX_ELEMENT_COUNT (64)
#define HALE_SCOPE_MAX_ELEMENT_LENGTH (255)
#define HALE_SCOPE_MAX_ELEMENT (HALE_SCOPE_MAX_ELEMENT_COUNT-1)

struct ScopeStatePath
{
    u8* path_names[HALE_SCOPE_MAX_ELEMENT_COUNT];
    u8 path_count;
};

struct ScopeObjectPath
{
    void *path_objects[HALE_SCOPE_MAX_ELEMENT_COUNT];
    u8 path_count;
};

struct Scope
{
    // TODO: We need to separate the names from the object pointers.
    ScopeStatePath path;
//    u8* path_names[HALE_SCOPE_MAX_ELEMENT_COUNT];
//    void *path_objects[HALE_SCOPE_MAX_ELEMENT_COUNT];
//    u8 path_count;

    template<typename T>
    T top(const char *name) { return 0; }

    ScopeSelectorData **selectors;
    memi selectors_count;
};

void scope_push(Set *set, Scope *scope, const char *name, void *object);

struct SelectorMatch
{
    typedef u8 Value[HALE_SCOPE_MAX_ELEMENT];
    memi top;
    Value value;
};

inline s32
match_compare(SelectorMatch *A, SelectorMatch *B)
{
    // 10000 > 10
    if (A->top > B->top) {
        return 1;
    }

    if (A->top < B->top) {
        return -1;
    }

    // 3001 > 3001
    // 5000 > 4000
    return memcmp((A->value + HALE_SCOPE_MAX_ELEMENT - A->top),
                  (B->value + HALE_SCOPE_MAX_ELEMENT - A->top),
                  A->top + 1);
}

inline void
match_plus(SelectorMatch *match, memi index, memi part) {
    hale_assert_input(index < HALE_SCOPE_MAX_ELEMENT_COUNT);
    hale_assert_input(part < HALE_SCOPE_MAX_ELEMENT_LENGTH);
    match->value[HALE_SCOPE_MAX_ELEMENT - index] = (u8)part;
    match->top = maximum(index, match->top);
}


struct ScopeArena
{
    // Interned keys.

    u8 *k_key_down;
    u8 *k_key_up;
    u8 *k_text;
    u8 *k_configure;

    enum {
        Tag_String,
        Tag_Selector
    };

    // Keeps unique set of strings and selectors.
    Set intern;

    // Keeps selectors, handlers and properties back to back.
    // Also keeps scope's matching selectors.
    PagedMemory data;
    // Keeps array of ScopeSelectorData.
    PagedMemory selectors;
    // Stores final Scope structs back to back.
    PagedMemory scopes;

    inline void operator ()(const char *selector, std::initializer_list<ConfigureProperty> properties);
};

#define hale_configure(C, selector, ...)\
    { const ConfigureKeyArg args[] = { __VA_ARGS__ };\
      configure(C, selector, (ConfigureKeyArg*)args, hale_array_count(args))\
    }

void selector_data_get(ScopeArena *A, ScopeSelector *selector, const ch8 *scope);

// Initializes the scope arena.
void scope_arena(ScopeArena *A);
// Returns Scope for given path defined by [it..end]
// Scope *scope_get(HALE_STACK, ScopeArena *A, Scope::E *it, Scope::E *end);

// Sets properties for scopes matching the `selector`.
void scope_set(ScopeArena *C,
               const char *selector,
               const ConfigureProperty *properties,
               memi properties_count);

inline void
ScopeArena::operator ()(const char *selector, std::initializer_list<ConfigureProperty> properties)
{
    scope_set(this, selector, properties.begin(), properties.size());
}

//
// Helpers
//

namespace configuration
{
    KeyEventHandler key(Key key, const EventHandler &handler)
    {
        KeyEventHandler ret;
        ret.key = key;
        ret.handler = handler;
        return ret;
    }

    KeyEventHandler text(const EventHandler &handler)
    {
        KeyEventHandler ret;
        ret.key.modifiers = 0;
        ret.key.codepoint = HALE_U32_MAX;
        ret.handler = handler;
        return ret;
    }

#define _MAKE_PROPERTY_HELPER_(ValueInputType, ValueStorageType, ValueCast) \
    template<typename Ch>\
    static ConfigureProperty property(const Ch *scope, const Ch *key, ValueInputType value)\
    {\
        static_assert(sizeof(Ch) == sizeof(ch8), "Invalid key type.");\
        ConfigureProperty ret;\
        ret.scope = (ch8*)scope;\
        ret.key = (ch8*)key;\
        ret.value.##ValueStorageType = ValueCast value;\
        ret.value.type = ConfigureValue::Type_##ValueStorageType;\
        return ret;\
    }

#define _MAKE_PROPERTY_HELPER(ValueInputType, ValueStorageType) \
    _MAKE_PROPERTY_HELPER_(ValueInputType, ValueStorageType,)

    _MAKE_PROPERTY_HELPER(Margin<r32>, Margin)
    _MAKE_PROPERTY_HELPER(Color32, Color)
    _MAKE_PROPERTY_HELPER(r32, Real32)
    _MAKE_PROPERTY_HELPER(r64, Real64)
    _MAKE_PROPERTY_HELPER(s32, Signed32)
    _MAKE_PROPERTY_HELPER(u32, Unsigned32)
    _MAKE_PROPERTY_HELPER(s64, Signed64)
    _MAKE_PROPERTY_HELPER(u64, Unsigned64)
    _MAKE_PROPERTY_HELPER(KeyEventHandler, KeyEventHandler)
    _MAKE_PROPERTY_HELPER(ConfigureEventHandler, ConfigureEventHandler)

    _MAKE_PROPERTY_HELPER_(const char *, String, (ch8*))

#undef _MAKE_PROPERTY_HELPER_
#undef _MAKE_PROPERTY_HELPER

} // namespace configuration

} // namespace hale

#endif // HALE_CONFIGURATION_H
