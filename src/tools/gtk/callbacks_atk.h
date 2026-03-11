// ----- AtkObjectClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(AtkObject, get_name, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_description, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_parent, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_n_children, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_child, void*, (void* accessible, int i), "pi", accessible, i);
WRAPPER(AtkObject, get_index_in_parent, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_relation_set, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_role, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_layer, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_mdi_zorder, int, (void* accessible), "p", accessible);
WRAPPER(AtkObject, ref_state_set, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, set_name, void, (void* accessible, void* name), "pp", accessible, name);
WRAPPER(AtkObject, set_description, void, (void* accessible, void* description), "pp", accessible, description);
WRAPPER(AtkObject, set_parent, void, (void* accessible, void* parent), "pp", accessible, parent);
WRAPPER(AtkObject, set_role, void, (void* accessible, int role), "pi", accessible, role);
WRAPPER(AtkObject, connect_property_change_handler, uint32_t, (void* accessible, void* handler), "pp", accessible, AddCheckBridge(my_bridge, vFpp, handler, 0, NULL));
WRAPPER(AtkObject, remove_property_change_handler, void, (void* accessible, uint32_t handler_id), "pu", accessible, handler_id);
WRAPPER(AtkObject, initialize, void, (void* accessible, void* data), "pp", accessible, data);
WRAPPER(AtkObject, children_changed, void, (void* accessible, uint32_t change_index, void* changed_child), "pup", accessible, change_index, changed_child);
WRAPPER(AtkObject, focus_event, void, (void* accessible, int focus_in), "pi", accessible, focus_in);
WRAPPER(AtkObject, property_change, void, (void* accessible, void* values), "pp", accessible, values);
WRAPPER(AtkObject, state_change, void, (void* accessible, void* name, int state_set), "ppi", accessible, name, state_set);
WRAPPER(AtkObject, visible_data_changed, void, (void* accessible), "p", accessible);
WRAPPER(AtkObject, active_descendant_changed, void, (void* accessible, void* child), "pp", accessible, child);
WRAPPER(AtkObject, get_attributes, void*, (void* accessible), "p", accessible);
WRAPPER(AtkObject, get_object_locale, void*, (void* accessible), "p", accessible);

#define SUPERGO() \
    GO(get_name, pFp);                          \
    GO(get_description, pFp);                   \
    GO(get_parent, pFp);                        \
    GO(get_n_children, iFp);                    \
    GO(ref_child, pFpi);                        \
    GO(get_index_in_parent, iFp);               \
    GO(ref_relation_set, pFp);                  \
    GO(get_role, iFp);                          \
    GO(get_layer, iFp);                         \
    GO(get_mdi_zorder, iFp);                    \
    GO(ref_state_set, pFp);                     \
    GO(set_name, vFpp);                         \
    GO(set_description, vFpp);                  \
    GO(set_parent, vFpp);                       \
    GO(set_role, vFpi);                         \
    GO(connect_property_change_handler, uFpp);  \
    GO(remove_property_change_handler, vFpu);   \
    GO(initialize, vFpp);                       \
    GO(children_changed, vFpup);                \
    GO(focus_event, vFpi);                      \
    GO(property_change, vFpp);                  \
    GO(state_change, vFppi);                    \
    GO(visible_data_changed, vFp);              \
    GO(active_descendant_changed, vFpp);        \
    GO(get_attributes, pFp);                    \
    GO(get_object_locale, pFp);                 \

// wrap (so bridge all calls, just in case)
static void wrapAtkObjectClass(my_AtkObjectClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapAtkObjectClass(my_AtkObjectClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeAtkObjectClass(my_AtkObjectClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_AtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapAtkObjectInstance(my_AtkObject_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeAtkObjectInstance(my_AtkObject_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- AtkUtilClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(AtkUtil,add_global_event_listener, uint32_t,(void* listener, void* event_type), "pp", AddCheckBridge(my_bridge, iFpupp, listener, 0, NULL), event_type);
WRAPPER(AtkUtil,remove_global_event_listener, void ,(uint32_t listener_id), "u", listener_id);
WRAPPER(AtkUtil,add_key_event_listener, uint32_t   ,(void* listener, void* data), "pp", AddCheckBridge(my_bridge, iFpp, listener, 0, NULL), data);
WRAPPER(AtkUtil,remove_key_event_listener, void    ,(uint32_t listener_id), "u", listener_id);
WRAPPER(AtkUtil,get_root, void*                    ,(void), "", 0);
WRAPPER(AtkUtil,get_toolkit_name, void*            ,(void), "", 0);
WRAPPER(AtkUtil,get_toolkit_version, void*         ,(void), "", 0);

#define SUPERGO() \
    GO(add_global_event_listener, uFpp);        \
    GO(remove_global_event_listener, vFu);      \
    GO(add_key_event_listener, uFpp);           \
    GO(remove_key_event_listener, vFu);         \
    GO(get_root, pFv);                          \
    GO(get_toolkit_name, pFv);                  \
    GO(get_toolkit_version, pFv);               \

// wrap (so bridge all calls, just in case)
static void wrapAtkUtilClass(my_AtkUtilClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapAtkUtilClass(my_AtkUtilClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeAtkUtilClass(my_AtkUtilClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_AtkUtil (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapAtkUtilInstance(my_AtkUtil_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeAtkUtilInstance(my_AtkUtil_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

