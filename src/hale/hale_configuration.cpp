#if HALE_INCLUDES
#include "hale_macros.h"
#include "hale_types.h"
#include "hale_memory.h"
#include "hale_string.h"
#include "hale_set.h"
#include "hale_configuration.h"
#endif

/*
 * - Configuration is set by calling configure(Configuration, ScopeSelector, Properties[]).
 * - System sorts out the properties by type into ConfigurationSelector struct.
 *   - Each event type has it's own array there.
 *   - Non-even properties are kept in one array in the struct.
 *   - Currently, system keeps one ConfigurationSelector per each call to this function,
 *     so it's better to make less calls. Further optimizations will merge the selectors together.
 * - When the configuration is done, application can request ConfigurationScope.
 *   Each such scope contains a final set of events and properties
 *   built from matching ConfigurationSelector::selector against the ConfigurationScope.
 * - All strings within the properties are interned, which takes a bit of performance hit, when
 *   the configuration is being changed, but it's fast when it's read.
 * - Further optimizations might keep the configuration cached completely on drive so it can
 *   be easily loaded on application start.
 */

namespace hale {

//
// Intern helpers.
//

hale_internal
inline u8 *
_intern_string(Set *S, u8 *begin, u8 *end)
{
    hale_assert(begin && end);
    hale_assert(begin < end);

    memi count = end - begin;
    hale_assert(count < 0xFF); // +1 for zero terminator

    memi ix;
    ch8 *ret = set_find(S, ScopeArena::Tag_String, begin, end);
    if (ret == 0) {
        ret = set_add(S, ScopeArena::Tag_String, begin, end, 0);
    }

    return ret;
}

hale_internal
inline u8 *
intern_string(Set *S, u8 *begin, u8 *end)
{
    if (set_contains(S, begin)) {
        return begin;
    }
    return _intern_string(S, begin, end);
}

hale_internal
inline u8 *
intern_string(Set *S, const u8 *string0)
{
    if (set_contains(S, (u8*)string0)) {
        return (ch8*)string0;
    }
    return _intern_string(S, (ch8*)string0, (ch8*)(string0 + string_length(string0)));
}

hale_internal
inline u8 *
intern_string(Set *S, const char *string0)
{
    return intern_string(S, (u8*)string0);
}

//
// Selectors
//

hale_internal
void
_selector_parse(Set *intern,
                PagedMemory *memory,
                ScopeSelector *selector,
                u8 *it)
{
    u8 *it_text = it;
    u8 **it_selector;

    selector->names = hale_memory_end(ch8*, memory);

    for (;; ++it)
    {
        if (*it == 0 || *it == ' ')
        {
            it_selector = hale_memory_push(ch8*, memory, 1);
            *it_selector = intern_string(intern, it_text, it);

            if (*it == 0) {
                break;
            }

            do {
                it++;
            } while (*it && *it == ' ');

            it_text = it;
        }
    }

    memi count = hale_memory_end(ch8*, memory) - selector->names;
    hale_assert_requirement(count <= 0xFF);
    selector->count = (u8)count;
}

ScopeSelectorData *
selector_data_find(ScopeArena *A, u8 *text)
{
    hale_assert_input(text && *text);

    memi index;
    ScopeSelectorData *selector_data;
    u8 *selector_end;
    u8 *text_end = text + string_length(text);
    if (set_find(&A->intern,
                 ScopeArena::Tag_Selector,
                 text, text_end,
                 &index,
                 (void**)&selector_data))
    {
        hale_assert_requirement(selector_data);
        return selector_data;
    }

    return NULL;
}

ScopeSelectorData *
selector_data_add(ScopeArena *A, u8 *text)
{
    hale_assert_input(text && *text);

    ScopeSelectorData *selector_data =
            hale_memory_push(ScopeSelectorData, &A->selectors, 1);

    *selector_data = {};
    _selector_parse(&A->intern, &A->data, &selector_data->selector, text);

    set_add(&A->intern, ScopeArena::Tag_Selector,
            (u8*)selector_data->selector.names,
            (u8*)(selector_data->selector.names + selector_data->selector.count),
            &selector_data);

    return selector_data;
}

ScopeSelectorData *
selector_data_get(ScopeArena *A, u8 *text)
{
    hale_assert_input(text && *text);

    ScopeSelectorData *selector_data = selector_data_find(A, text);
    if (selector_data == NULL) {
        selector_data = selector_data_add(A, text);
    }
    return selector_data;
}

//
// Scopes
//

void
scope_arena(ScopeArena *A)
{
    set_init(&A->intern, hale_megabytes(1));

    memory_init(&A->data,               hale_megabytes(1), 0);
    memory_init(&A->selectors,          hale_megabytes(1), 0);
    memory_init(&A->scopes,             hale_megabytes(1), 0);

    A->k_key_down  = intern_string(&A->intern, "_key_down");
    A->k_key_up    = intern_string(&A->intern, "_key_up");
    A->k_text      = intern_string(&A->intern, "_text");
    A->k_configure = intern_string(&A->intern, "_configure");
}

#if 0
// TODO: Again a memory_compare.
hale_internal
inline memi
_scope_path_compare(Scope::E *a_it, Scope::E *a_end, Scope::E *b_it, Scope::E *b_end)
{
    while (*a_it == *b_it && a_it != a_end && b_it != b_end)
    {
        a_it++;
        b_it++;
    }
    return a_end - a_it;
}

hale_internal b32
_scope_path_match(Scope::E *it,
                  Scope::E *end,
                  ScopeSelector *selector,
                  SelectorMatch *match)
{
    return 0;
}

void
scope_push(Set *set, Scope *scope, const char *name, void *object)
{
    hale_assert(scope->path_count != HALE_SCOPE_MAX_ELEMENT_COUNT);
    scope->path_names[scope->path_count] = intern_string(set, name);
    scope->path_names[scope->path_count] = object;
    scope->path_count++;
}

// NOTE: This is bottle neck for reusing the existing scopes.
Scope *
scope_find(ScopeArena *A, Scope::E *it, Scope::E *end)
{
    memi match_full = end - it;
    Scope *s_it  = hale_memory_begin(Scope, A->scopes);
    Scope *s_end = hale_memory_end(Scope, A->scopes);
    for (; s_it != s_end; s_it++) {
        if (_scope_path_compare(s_it->path, s_it->path + s_it->path_count, it, end) == match_full) {
            return s_it;
        }
    }

    return 0;
}

// NOTE: This is a possible bottle neck in case the new scope paths
//       will appear too often.
Scope *
scope_make(HALE_STACK, ScopeArena *A, Scope::E *it, Scope::E *end)
{
    auto scope = hale_memory_push(Scope, &A->scopes, 1);

    scope->selectors = hale_memory_end(ScopeSelectorData*, &A->data);

    StackMemory<SelectorMatch> matches(stack);
    SelectorMatch *match = matches.push(1);

    ScopeSelectorData **d;
    auto selector_it  = hale_memory_begin(ScopeSelectorData, &A->selectors);
    auto selector_end = hale_memory_end(ScopeSelectorData, &A->selectors);
    for (; selector_it != selector_end; ++selector_it)
    {
        if (_scope_path_match(it, end, selector_it, &match)) {
            d = hale_memory_push(ScopeSelectorData*, &A->data, 1);
            *d = selector_it;
            match = matches.push(1);
        }
    }
    scope->selectors_count = hale_memory_end(ScopeSelectorData*, &A->data) - scope->selectors;

    // TODO: We have all matched selectors in match(), now we need to sort them.
    //     - Do we really need to sort them?
    //     - See the TODO above, because we'll have to sort selectors array by scores array.

    return scope;
}

Scope *
scope_get(HALE_STACK, ScopeArena *A, Scope::E *it, Scope::E *end)
{
    Scope *scope = scope_find(A, it, end);
    if (scope == 0) {
        scope = scope_make(stack, A, it, end);
    }
    return scope;
}
#endif
//
// Configuration helpers.
//

hale_internal memi
_add_key_event_handler(ScopeArena *A,
                       ch8 *key,
                       ConfigureProperty *it,
                       ConfigureProperty *end,
                       KeyEventHandler **handlers)
{
    memi count = 0;
    *handlers = hale_memory_end(KeyEventHandler, &A->data);
    KeyEventHandler *handler;
    for (; it != end; ++it)
    {
        if (it->key == key && it->value.type == ConfigureValue::Type_KeyEventHandler)
        {
            handler = hale_memory_push(KeyEventHandler, &A->data, 1);
            *handler = it->value.KeyEventHandler;
            ++count;
        }
    }
    return count;
}

hale_internal memi
_add_configure_event_handler(ScopeArena *C,
                       ch8 *key,
                       ConfigureProperty *it,
                       ConfigureProperty *end,
                       ConfigureEventHandler **handlers)
{
    memi count = 0;
    *handlers = hale_memory_end(ConfigureEventHandler, &C->data);
    ConfigureEventHandler *handler;
    for (; it != end; ++it)
    {
        if (it->key == key && it->value.type == ConfigureValue::Type_ConfigureEventHandler)
        {
            handler = hale_memory_push(ConfigureEventHandler, &C->data, 1);
            handler->handler = it->value.ConfigureEventHandler.handler;
            ++count;
        }
    }
    return count;
}

void
scope_set(ScopeArena *A,
          const char *selector_text,
          const ConfigureProperty *properties,
          memi properties_count)
{
    ScopeSelectorData *selector = selector_data_get(A, (u8*)selector_text);

    ConfigureProperty *begin = (ConfigureProperty *)properties;
    ConfigureProperty *end = begin + properties_count;

#define _ADD_KEY_EVENT_HANDLER(event, fn) \
    selector->data.##event##_count \
            = _add_##fn##_event_handler(A,\
                                     A->k_##event,\
                                     begin,\
                                     end,\
                                     &selector->data.##event)

    _ADD_KEY_EVENT_HANDLER(key_down, key);
    _ADD_KEY_EVENT_HANDLER(key_up, key);
    _ADD_KEY_EVENT_HANDLER(text, key);
    _ADD_KEY_EVENT_HANDLER(configure, configure);

#undef _ADD_KEY_EVENT_HANDLER

    selector->data.properties = hale_memory_end(ConfigureProperty, &A->data);
    ConfigureProperty *property;
    auto it = begin;
    for (; it != end; it++)
    {
        if (it->key[0] != '_')
        {
            property = hale_memory_push(ConfigureProperty, &A->data, 1);
            property->key = intern_string(&A->intern, it->key);
			// TODO: We need to deal with the string values.
            property->value = it->value;
            ++selector->data.properties_count;
        }
    }
}

}
