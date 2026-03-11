// ----- GApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GApplication, startup, void, (void* application), "p", application);
WRAPPER(GApplication, activate, void, (void* application), "p", application);
WRAPPER(GApplication, open, void, (void* application, void* files, int n_files, void* hint), "ppip", application, files, n_files, hint);
WRAPPER(GApplication, command_line, void, (void* application, void* command_line), "pp", application, command_line);
WRAPPER(GApplication, local_command_line, void, (void* application, void* arguments, void* exit_status), "ppp", application, arguments, exit_status);
WRAPPER(GApplication, before_emit, void*, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, after_emit, void, (void* application, void* platform_data), "pp", application, platform_data);
WRAPPER(GApplication, add_platform_data, void, (void* application, void* builder), "pp", application, builder);
WRAPPER(GApplication, quit_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, run_mainloop, void, (void* application), "p", application);
WRAPPER(GApplication, shutdown, void, (void* application), "p", application);
WRAPPER(GApplication, dbus_register, void, (void* application, void* connection, void* object_path, void* error), "pppp", application, connection, object_path, error);
WRAPPER(GApplication, dbus_unregister, void, (void* application, void* connection, void* object_path), "ppp", application, connection, object_path);
WRAPPER(GApplication, handle_local_options, void, (void* application, void* options), "pp", application, options);
WRAPPER(GApplication, name_lost, void, (void* application), "p", application);

#define SUPERGO()                     \
    GO(startup, vFp);                 \
    GO(activate, vFp);                \
    GO(open, vFppip);                 \
    GO(command_line, vFpp);           \
    GO(local_command_line, vFppp);    \
    GO(before_emit, vFpp);            \
    GO(after_emit, vFpp);             \
    GO(add_platform_data, vFpp);      \
    GO(quit_mainloop, vFp);           \
    GO(run_mainloop, vFp);            \
    GO(shutdown, vFp);                \
    GO(dbus_register, vFpppp);        \
    GO(dbus_unregister, vFppp);       \
    GO(handle_local_options, vFpp);   \
    GO(name_lost, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGApplicationClass(my_GApplicationClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGApplicationClass(my_GApplicationClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGApplicationClass(my_GApplicationClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGApplicationInstance(my_GApplication_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGApplicationInstance(my_GApplication_t* class)
{
    bridgeGObjectInstance(&class->parent);
}
#undef SUPERGO

// ----- GtkApplicationClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkApplication, window_added, void, (void* application, void* window), "pp", application, window);
WRAPPER(GtkApplication, window_removed, void, (void* application, void* window), "pp", application, window);

#define SUPERGO() \
    GO(window_added, pFpp);   \
    GO(window_removed, vFpp);

static void wrapGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    wrapGApplicationClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

static void unwrapGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    unwrapGApplicationClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

static void bridgeGtkApplicationClass(my_GtkApplicationClass_t* class)
{
    bridgeGApplicationClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkApplication (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkApplicationInstance(my_GtkApplication_t* class)
{
    unwrapGApplicationInstance(&class->parent);
}

static void bridgeGtkApplicationInstance(my_GtkApplication_t* class)
{
    bridgeGApplicationInstance(&class->parent);
}

// ----- GtkObjectClass ------
// wrapper x64 -> natives of callbacks
WRAPPER(GtkObject, set_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, get_arg, void, (void* object, void* arg, uint32_t arg_id), "ppu", object, arg, arg_id);
WRAPPER(GtkObject, destroy, void, (void* object), "p", object);

#define SUPERGO() \
    GO(set_arg, vFppu); \
    GO(get_arg, vFppu); \
    GO(destroy, vFp);
// wrap (so bridge all calls, just in case)
static void wrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkObjectClass(my_GtkObjectClass_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkObjectClass(my_GtkObjectClass_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkObject (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkObjectInstance(my_GtkObject_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
static void bridgeGtkObjectInstance(my_GtkObject_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GtkWidget2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget2, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget2, show,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, show_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide,              void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, hide_all,          void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, map,               void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unmap,             void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, realize,           void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, unrealize,         void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, size_request,      void, (void* widget, void* requisition), "pp", widget, requisition);
WRAPPER(GtkWidget2, size_allocate,     void, (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget2, state_changed,     void, (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget2, parent_set,        void, (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget2, hierarchy_changed, void, (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget2, style_set,         void, (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget2, direction_changed, void, (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget2, grab_notify,       void, (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget2, child_notify,      void, (void* widget, void* pspec), "pp", widget, pspec);
WRAPPER(GtkWidget2, mnemonic_activate, int, (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget2, grab_focus,        void, (void* widget), "p", widget);
WRAPPER(GtkWidget2, focus,             int, (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget2, event,             int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_press_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, button_release_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, scroll_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, motion_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, delete_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, destroy_event,      int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, expose_event,       int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_press_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, key_release_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, enter_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, leave_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, configure_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_in_event,     int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, focus_out_event,    int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, map_event,          int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, unmap_event,        int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, property_notify_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_clear_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_request_event,int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, selection_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_in_event,  int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, proximity_out_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2, visibility_notify_event, int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, client_event,        int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, no_expose_event,     int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, window_state_event,  int, (void* widget, void* event), "p", widget, event);
WRAPPER(GtkWidget2, selection_get,       void, (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget2, selection_received,  void, (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget2, drag_begin,          void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_end,            void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_data_get,       void, (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget2, drag_data_delete,    void, (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget2, drag_leave,          void, (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget2, drag_motion,         int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_drop,           int, (void* widget, void* context, int32_t x, int32_t y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget2, drag_data_received,  void, (void* widget, void* context, int32_t x, int32_t y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget2,  popup_menu,         int  , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  show_help,          int  , (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget2, get_accessible,      void*, (void* widget), "p", widget);
WRAPPER(GtkWidget2, screen_changed,      void , (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget2, can_activate_accel,  int  , (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget2, grab_broken_event,   int  , (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget2,  composited_changed, void , (void* widget), "p", widget);
WRAPPER(GtkWidget2,  query_tooltip,      int  , (void* widget, int32_t x, int32_t y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(hide_all, vFp);                              \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_request, vFpp);                         \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(expose_event, iFpp);                         \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(client_event, iFpp);                         \
    GO(no_expose_event, iFpp);                      \
    GO(window_state_event, iFpp);                   \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);

// wrap (so bridge all calls, just in case)
static void wrapGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWidget2Class(my_GtkWidget2Class_t* class)
{
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWidget2Instance(my_GtkWidget2_t* class)
{
    unwrapGtkObjectInstance(&class->private);
}
// autobridge
static void bridgeGtkWidget2Instance(my_GtkWidget2_t* class)
{
    bridgeGtkObjectInstance(&class->private);
}

// ----- GtkWidget3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWidget3, dispatch_child_properties_changed, void, (void* widget, uint32_t n_pspecs, void* pspecs), "pup", widget, n_pspecs, pspecs);
WRAPPER(GtkWidget3, destroy, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, show, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_all, void,             (void* widget), "p", widget);
WRAPPER(GtkWidget3, hide, void,                 (void* widget), "p", widget);
WRAPPER(GtkWidget3, map, void,                  (void* widget), "p", widget);
WRAPPER(GtkWidget3, unmap, void,                (void* widget), "p", widget);
WRAPPER(GtkWidget3, realize, void,              (void* widget), "p", widget);
WRAPPER(GtkWidget3, unrealize, void,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, size_allocate, void,        (void* widget, void* allocation), "pp", widget, allocation);
WRAPPER(GtkWidget3, state_changed, void,        (void* widget, int previous_state), "pi", widget, previous_state);
WRAPPER(GtkWidget3, state_flags_changed, void,  (void* widget, int previous_state_flags), "pi", widget, previous_state_flags);
WRAPPER(GtkWidget3, parent_set, void,           (void* widget, void* previous_parent), "pp", widget, previous_parent);
WRAPPER(GtkWidget3, hierarchy_changed, void,    (void* widget, void* previous_toplevel), "pp", widget, previous_toplevel);
WRAPPER(GtkWidget3, style_set, void,            (void* widget, void* previous_style), "pp", widget, previous_style);
WRAPPER(GtkWidget3, direction_changed, void,    (void* widget, int previous_direction), "pi", widget, previous_direction);
WRAPPER(GtkWidget3, grab_notify, void,          (void* widget, int was_grabbed), "pi", widget, was_grabbed);
WRAPPER(GtkWidget3, child_notify, void,         (void* widget, void* child_property), "pp", widget, child_property);
WRAPPER(GtkWidget3, draw, int,                  (void* widget, void* cr), "pp", widget, cr);
WRAPPER(GtkWidget3, get_request_mode, int,      (void* widget), "p", widget);
WRAPPER(GtkWidget3, get_preferred_height, void, (void* widget, void* minimum_height, void* natural_height), "ppp", widget, minimum_height, natural_height);
WRAPPER(GtkWidget3, get_preferred_width_for_height, void,  (void* widget, int height, void* minimum_width, void* natural_width), "pipp", widget, height, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_width, void,  (void* widget, void* minimum_width, void* natural_width), "ppp", widget, minimum_width, natural_width);
WRAPPER(GtkWidget3, get_preferred_height_for_width, void,  (void* widget, int width, void* minimum_height, void* natural_height), "pipp", widget, width, minimum_height, natural_height);
WRAPPER(GtkWidget3, mnemonic_activate, int,     (void* widget, int group_cycling), "pi", widget, group_cycling);
WRAPPER(GtkWidget3, grab_focus, void,           (void* widget), "p", widget);
WRAPPER(GtkWidget3, focus, int,                 (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, move_focus, void,           (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, keynav_failed, int,         (void* widget, int direction), "pi", widget, direction);
WRAPPER(GtkWidget3, event, int,                 (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_press_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, button_release_event, int,  (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, scroll_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, motion_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, delete_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, destroy_event, int,         (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_press_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, key_release_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, enter_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, leave_notify_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, configure_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_in_event, int,        (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, focus_out_event, int,       (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, map_event, int,             (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, unmap_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, property_notify_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_clear_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_request_event, int, (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_notify_event, int,(void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_in_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, proximity_out_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, visibility_notify_event, int,   (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, window_state_event, int,    (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, damage_event, int,          (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, grab_broken_event, int,     (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, selection_get, void,        (void* widget, void* selection_data, uint32_t info, uint32_t time_), "ppuu", widget, selection_data, info, time_);
WRAPPER(GtkWidget3, selection_received, void,   (void* widget, void* selection_data, uint32_t time_), "ppu", widget, selection_data, time_);
WRAPPER(GtkWidget3, drag_begin, void,           (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_end, void,             (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_data_get, void,        (void* widget, void* context, void* selection_data, uint32_t info, uint32_t time_), "pppuu", widget, context, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_data_delete, void,     (void* widget, void* context), "pp", widget, context);
WRAPPER(GtkWidget3, drag_leave, void,           (void* widget, void* context, uint32_t time_), "ppu", widget, context, time_);
WRAPPER(GtkWidget3, drag_motion, int,           (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_drop, int,             (void* widget, void* context, int x, int y, uint32_t time_), "ppiiu", widget, context, x, y, time_);
WRAPPER(GtkWidget3, drag_data_received, void,   (void* widget, void* context, int x, int y, void* selection_data, uint32_t info, uint32_t time_), "ppiipuu", widget, context, x, y, selection_data, info, time_);
WRAPPER(GtkWidget3, drag_failed, int,           (void* widget, void* context, int result), "ppi", widget, context, result);
WRAPPER(GtkWidget3, popup_menu, int,            (void* widget), "p", widget);
WRAPPER(GtkWidget3, show_help, int,             (void* widget, int help_type), "pi", widget, help_type);
WRAPPER(GtkWidget3, get_accessible, void*,      (void *widget), "p", widget);
WRAPPER(GtkWidget3, screen_changed, void,       (void* widget, void* previous_screen), "pp", widget, previous_screen);
WRAPPER(GtkWidget3, can_activate_accel, int,    (void* widget, uint32_t signal_id), "pu", widget, signal_id);
WRAPPER(GtkWidget3, composited_changed, void,   (void* widget), "p", widget);
WRAPPER(GtkWidget3, query_tooltip, int,         (void* widget, int x, int y, int keyboard_tooltip, void* tooltip), "piiip", widget, x, y, keyboard_tooltip, tooltip);
WRAPPER(GtkWidget3, compute_expand, void,       (void* widget, void* hexpand_p, void* vexpand_p), "ppp", widget, hexpand_p, vexpand_p);
WRAPPER(GtkWidget3, adjust_size_request, void,  (void* widget, int orientation, void* minimum_size, void* natural_size), "pipp", widget, orientation, minimum_size, natural_size);
WRAPPER(GtkWidget3, adjust_size_allocation, void, (void*widget, int orientation, void* minimum_size, void* natural_size, void* allocated_pos, void* allocated_size), "pipppp", widget, orientation, minimum_size, natural_size, allocated_pos, allocated_size);
WRAPPER(GtkWidget3, style_updated, void,        (void* widget), "p", widget);
WRAPPER(GtkWidget3, touch_event, int,           (void* widget, void* event), "pp", widget, event);
WRAPPER(GtkWidget3, get_preferred_height_and_baseline_for_width, void, (void* widget, int width, void* minimum_height, void* natural_height, void* minimum_baseline, void* natural_baseline), "pipppp", widget, width, minimum_height, natural_height, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_request, void,  (void* widget, void* minimum_baseline, void* natural_baseline), "ppp", widget, minimum_baseline, natural_baseline);
WRAPPER(GtkWidget3, adjust_baseline_allocation, void,  (void* widget, void* baseline), "pp", widget, baseline);
WRAPPER(GtkWidget3, queue_draw_region, void,    (void* widget, void* region), "pp", widget, region);

#define SUPERGO() \
    GO(dispatch_child_properties_changed, vFpup);   \
    GO(destroy, vFp);                               \
    GO(show, vFp);                                  \
    GO(show_all, vFp);                              \
    GO(hide, vFp);                                  \
    GO(map, vFp);                                   \
    GO(unmap, vFp);                                 \
    GO(realize, vFp);                               \
    GO(unrealize, vFp);                             \
    GO(size_allocate, vFpp);                        \
    GO(state_changed, vFpi);                        \
    GO(state_flags_changed, vFpi);                  \
    GO(parent_set, vFpp);                           \
    GO(hierarchy_changed, vFpp);                    \
    GO(style_set, vFpp);                            \
    GO(direction_changed, vFpi);                    \
    GO(grab_notify, vFpi);                          \
    GO(child_notify, vFpp);                         \
    GO(draw, iFpp);                                 \
    GO(get_request_mode, iFp);                      \
    GO(get_preferred_height, vFppp);                \
    GO(get_preferred_width_for_height, vFpipp);     \
    GO(get_preferred_width, vFppp);                 \
    GO(get_preferred_height_for_width, vFpipp);     \
    GO(mnemonic_activate, iFpi);                    \
    GO(grab_focus, vFp);                            \
    GO(focus, iFpi);                                \
    GO(move_focus, vFpi);                           \
    GO(keynav_failed, iFpi);                        \
    GO(event, iFpp);                                \
    GO(button_press_event, iFpp);                   \
    GO(button_release_event, iFpp);                 \
    GO(scroll_event, iFpp);                         \
    GO(motion_notify_event, iFpp);                  \
    GO(delete_event, iFpp);                         \
    GO(destroy_event, iFpp);                        \
    GO(key_press_event, iFpp);                      \
    GO(key_release_event, iFpp);                    \
    GO(enter_notify_event, iFpp);                   \
    GO(leave_notify_event, iFpp);                   \
    GO(configure_event, iFpp);                      \
    GO(focus_in_event, iFpp);                       \
    GO(focus_out_event, iFpp);                      \
    GO(map_event, iFpp);                            \
    GO(unmap_event, iFpp);                          \
    GO(property_notify_event, iFpp);                \
    GO(selection_clear_event, iFpp);                \
    GO(selection_request_event, iFpp);              \
    GO(selection_notify_event, iFpp);               \
    GO(proximity_in_event, iFpp);                   \
    GO(proximity_out_event, iFpp);                  \
    GO(visibility_notify_event, iFpp);              \
    GO(window_state_event, iFpp);                   \
    GO(damage_event, iFpp);                         \
    GO(grab_broken_event, iFpp);                    \
    GO(selection_get, vFppuu);                      \
    GO(selection_received, vFppu);                  \
    GO(drag_begin, vFpp);                           \
    GO(drag_end, vFpp);                             \
    GO(drag_data_get, vFpppuu);                     \
    GO(drag_data_delete, vFpp);                     \
    GO(drag_leave, vFppu);                          \
    GO(drag_motion, iFppiiu);                       \
    GO(drag_drop, iFppiiu);                         \
    GO(drag_data_received, vFppiipuu);              \
    GO(drag_failed, iFppi);                         \
    GO(popup_menu, iFp);                            \
    GO(show_help, iFpi);                            \
    GO(get_accessible, pFp);                        \
    GO(screen_changed, vFpp);                       \
    GO(can_activate_accel, iFpu);                   \
    GO(grab_broken_event, iFpp);                    \
    GO(composited_changed, vFp);                    \
    GO(query_tooltip, iFpiiip);                     \
    GO(compute_expand, vFppp);                      \
    GO(adjust_size_request, vFpipp);                \
    GO(adjust_size_allocation, vFpipppp);           \
    GO(style_updated, vFp);                         \
    GO(touch_event, iFpp);                          \
    GO(get_preferred_height_and_baseline_for_width, vFpipppp);\
    GO(adjust_baseline_request, vFppp);             \
    GO(adjust_baseline_allocation, vFpp);           \
    GO(queue_draw_region, vFpp);                    \

// wrap (so bridge all calls, just in case)
static void wrapGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    wrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    unwrapGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWidget3Class(my_GtkWidget3Class_t* class)
{
    bridgeGInitiallyUnownedClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWidget3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWidget3Instance(my_GtkWidget3_t* class)
{
    unwrapGInitiallyUnownedInstance(&class->parent);
}
// autobridge
static void bridgeGtkWidget3Instance(my_GtkWidget3_t* class)
{
    bridgeGInitiallyUnownedInstance(&class->parent);
}

// ----- GtkContainer2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer2, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer2, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer2, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer2, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer2, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer2, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer2, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);

// wrap (so bridge all calls, just in case)
static void wrapGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkContainer2Class(my_GtkContainer2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkContainer2Instance(my_GtkContainer2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
static void bridgeGtkContainer2Instance(my_GtkContainer2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkContainer3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkContainer3, add, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, remove, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, check_resize, void, (void* container), "p", container);
WRAPPER(GtkContainer3, forall, void, (void* container, int include_internals, void* callback, void* callback_data), "pipp", container, include_internals, AddCheckBridge(my_bridge, vFpp, callback, 0, NULL), callback_data);
WRAPPER(GtkContainer3, set_focus_child, void, (void* container, void* widget), "pp", container, widget);
WRAPPER(GtkContainer3, child_type, int, (void* container), "p", container);
WRAPPER(GtkContainer3, composite_name, void*, (void* container, void* child), "pp", container, child);
WRAPPER(GtkContainer3, set_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_child_property, void, (void* container, void* child, uint32_t property_id, void* value, void* pspec), "ppupp", container, child, property_id, value, pspec);
WRAPPER(GtkContainer3, get_path_for_child, void*, (void* container, void* child), "pp", container, child);

#define SUPERGO() \
    GO(add, vFpp);                  \
    GO(remove, vFpp);               \
    GO(check_resize, vFp);          \
    GO(forall, vFpipp);             \
    GO(set_focus_child, vFpp);      \
    GO(child_type, iFp);            \
    GO(composite_name, pFpp);       \
    GO(set_child_property, vFppupp);\
    GO(get_child_property, vFppupp);\
    GO(get_path_for_child, pFpp);   \

// wrap (so bridge all calls, just in case)
static void wrapGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkContainer3Class(my_GtkContainer3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkContainer3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkContainer3Instance(my_GtkContainer3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
static void bridgeGtkContainer3Instance(my_GtkContainer3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkActionClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkAction, activate, void, (void* action), "p", action);
WRAPPER(GtkAction, create_menu_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, create_tool_item, void*, (void* action), "p", action);
WRAPPER(GtkAction, connect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, disconnect_proxy, void , (void* action, void* proxy), "pp", action, proxy);
WRAPPER(GtkAction, create_menu, void*, (void* action), "p", action);

#define SUPERGO() \
    GO(activate, vFp);          \
    GO(create_menu_item, pFp);  \
    GO(create_tool_item, pFp);  \
    GO(connect_proxy, vFpp);    \
    GO(disconnect_proxy, vFpp); \
    GO(create_menu, pFp);       \

// wrap (so bridge all calls, just in case)
static void wrapGtkActionClass(my_GtkActionClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkActionClass(my_GtkActionClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkActionClass(my_GtkActionClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkAction (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGtkActionInstance(my_GtkAction_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGtkActionInstance(my_GtkAction_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkDrawingArea3Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkDrawingArea3Class(my_GtkDrawingArea3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkDrawingArea3Class(my_GtkDrawingArea3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkDrawingArea3Class(my_GtkDrawingArea3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
}

static void unwrapGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
static void bridgeGtkDrawingArea3Instance(my_GtkDrawingArea3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkMisc2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkMisc2Class(my_GtkMisc2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
}

static void unwrapGtkMisc2Instance(my_GtkMisc2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
static void bridgeGtkMisc2Instance(my_GtkMisc2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkMisc3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    wrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    unwrapGtkWidget3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMisc3Class(my_GtkMisc3Class_t* class)
{
    bridgeGtkWidget3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMisc3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkMisc3Instance(my_GtkMisc3_t* class)
{
    unwrapGtkWidget3Instance(&class->parent);
}
// autobridge
static void bridgeGtkMisc3Instance(my_GtkMisc3_t* class)
{
    bridgeGtkWidget3Instance(&class->parent);
}

// ----- GtkImage3Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkImage3Class(my_GtkImage3Class_t* class)
{
    wrapGtkMisc3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkImage3Class(my_GtkImage3Class_t* class)
{
    unwrapGtkMisc3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkImage3Class(my_GtkImage3Class_t* class)
{
    bridgeGtkMisc3Class(&class->parent_class);
}

static void unwrapGtkImage3Instance(my_GtkImage3_t* class)
{
    unwrapGtkMisc3Instance(&class->parent);
}
// autobridge
static void bridgeGtkImage3Instance(my_GtkImage3_t* class)
{
    bridgeGtkMisc3Instance(&class->parent);
}


// ----- GtkLabel2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel2, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel2, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel2, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel2, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    wrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    unwrapGtkMisc2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkLabel2Class(my_GtkLabel2Class_t* class)
{
    bridgeGtkMisc2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel2 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGtkLabel2Instance(my_GtkLabel2_t* class)
{
    unwrapGtkMisc2Instance(&class->misc);
}
// autobridge
static void bridgeGtkLabel2Instance(my_GtkLabel2_t* class)
{
    bridgeGtkMisc2Instance(&class->misc);
}

// ----- GtkLabel3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkLabel3, move_cursor, void, (void* label, int step, int count, int extend_selection), "piii", label, step, count, extend_selection);
WRAPPER(GtkLabel3, copy_clipboard, void, (void* label), "p", label);
WRAPPER(GtkLabel3, populate_popup, void, (void* label, void* menu), "pp", label, menu);
WRAPPER(GtkLabel3, activate_link, int, (void* label, void* uri), "pp", label, uri);

#define SUPERGO() \
    GO(move_cursor, vFpiii);    \
    GO(copy_clipboard, vFp);    \
    GO(populate_popup, vFpp);   \
    GO(activate_link, iFpp);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    wrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    unwrapGtkMisc3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkLabel3Class(my_GtkLabel3Class_t* class)
{
    bridgeGtkMisc3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkLabel3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkLabel3Instance(my_GtkLabel3_t* class)
{
    unwrapGtkMisc3Instance(&class->misc);
}
// autobridge
static void bridgeGtkLabel3Instance(my_GtkLabel3_t* class)
{
    bridgeGtkMisc3Instance(&class->misc);
}

// ----- GtkTreeView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTreeView2, set_scroll_adjustments, void, (void* tree_view, void* hadjustment, void* vadjustment), "ppp", tree_view, hadjustment, vadjustment);
WRAPPER(GtkTreeView2, row_activated, void, (void* tree_view, void* path, void* column), "ppp", tree_view, path, column);
WRAPPER(GtkTreeView2, test_expand_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, test_collapse_row, int, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_expanded, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, row_collapsed, void, (void* tree_view, void* iter, void* path), "ppp", tree_view, iter, path);
WRAPPER(GtkTreeView2, columns_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, cursor_changed, void, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, move_cursor, int, (void* tree_view, int step, int count), "pii", tree_view, step, count);
WRAPPER(GtkTreeView2, select_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, unselect_all, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, select_cursor_row, int, (void* tree_view, int start_editing), "pi", tree_view, start_editing);
WRAPPER(GtkTreeView2, toggle_cursor_row, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, expand_collapse_cursor_row, int, (void* tree_view, int logical, int expand, int open_all), "piii", tree_view, logical, expand, open_all);
WRAPPER(GtkTreeView2, select_cursor_parent, int, (void* tree_view), "p", tree_view);
WRAPPER(GtkTreeView2, start_interactive_search, int, (void* tree_view), "p", tree_view);

#define SUPERGO() \
    GO(set_scroll_adjustments, vFppp);      \
    GO(row_activated, vFppp);               \
    GO(test_expand_row, iFppp);             \
    GO(test_collapse_row, iFppp);           \
    GO(row_expanded, vFppp);                \
    GO(row_collapsed, vFppp);               \
    GO(columns_changed, vFp);               \
    GO(cursor_changed, vFp);                \
    GO(move_cursor, iFppp);                 \
    GO(select_all, iFp);                    \
    GO(unselect_all, iFp);                  \
    GO(select_cursor_row, iFpi);            \
    GO(toggle_cursor_row, iFp);             \
    GO(expand_collapse_cursor_row, iFpiii); \
    GO(select_cursor_parent, iFp);          \
    GO(start_interactive_search, iFp);      \

// wrap (so bridge all calls, just in case)
static void wrapGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTreeView2Class(my_GtkTreeView2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTreeView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkTreeView2Instance(my_GtkTreeView2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkTreeView2Instance(my_GtkTreeView2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkBin2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkBin2Class(my_GtkBin2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkBin2Class(my_GtkBin2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkBin2Class(my_GtkBin2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

static void unwrapGtkBin2Instance(my_GtkBin2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
static void bridgeGtkBin2Instance(my_GtkBin2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}

// ----- GtkBin3Class ------
static void wrapGtkBin3Class(my_GtkBin3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkBin3Class(my_GtkBin3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkBin3Class(my_GtkBin3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
}

static void unwrapGtkBin3Instance(my_GtkBin3_t* class)
{
    unwrapGtkContainer3Instance(&class->container);
}
// autobridge
static void bridgeGtkBin3Instance(my_GtkBin3_t* class)
{
    bridgeGtkContainer3Instance(&class->container);
}
// ----- GtkWindow2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow2, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow2, frame_event, int, (void* window, void* event), "pp", window, event);
WRAPPER(GtkWindow2, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow2, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow2, move_focus, void, (void* window, int direction), "pi", window, direction);
WRAPPER(GtkWindow2, keys_changed, void, (void* window), "p", window);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(frame_event, iFpp);      \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(move_focus, vFpi);       \
    GO(keys_changed, vFp);      \


// wrap (so bridge all calls, just in case)
static void wrapGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWindow2Class(my_GtkWindow2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWindow2Instance(my_GtkWindow2_t* class)
{
    unwrapGtkBin2Instance(&class->parent);
}
// autobridge
static void bridgeGtkWindow2Instance(my_GtkWindow2_t* class)
{
    bridgeGtkBin2Instance(&class->parent);
}

// ----- GtkWindow3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkWindow3, set_focus, void, (void* window, void* focus), "pp", window, focus);
WRAPPER(GtkWindow3, activate_focus, void, (void* window), "p", window);
WRAPPER(GtkWindow3, activate_default, void, (void* window), "p", window);
WRAPPER(GtkWindow3, keys_changed, void, (void* window), "p", window);
WRAPPER(GtkWindow3, enable_debugging, int, (void* window, int toggle), "pi", window, toggle);

#define SUPERGO()               \
    GO(set_focus, vFpp);        \
    GO(activate_focus, vFp);    \
    GO(activate_default, vFp);  \
    GO(keys_changed, vFp);      \
    GO(enable_debugging, iFpi);


// wrap (so bridge all calls, just in case)
static void wrapGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkWindow3Class(my_GtkWindow3Class_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkWindow3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkWindow3Instance(my_GtkWindow3_t* class)
{
    unwrapGtkBin3Instance(&class->bin);
}
// autobridge
static void bridgeGtkWindow3Instance(my_GtkWindow3_t* class)
{
    bridgeGtkBin3Instance(&class->bin);
}

// ----- GtkApplicationWindowClass ------
// wrap (so bridge all calls, just in case)
static void wrapGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    wrapGtkWindow3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    unwrapGtkWindow3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkApplicationWindowClass(my_GtkApplicationWindowClass_t* class)
{
    bridgeGtkWindow3Class(&class->parent_class);
}

static void unwrapGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class)
{
    unwrapGtkWindow3Instance(&class->parent);
}
// autobridge
static void bridgeGtkApplicationWindowInstance(my_GtkApplicationWindow_t* class)
{
    bridgeGtkWindow3Instance(&class->parent);
}
// ----- GtkListBoxClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxClass,row_selected, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,row_activated, void, (void *box, void *row), "pp", box, row);
WRAPPER(GtkListBoxClass,activate_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,toggle_cursor_row, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,move_cursor, void, (void *box, int step, int count), "pii", box, step, count);
WRAPPER(GtkListBoxClass,selected_rows_changed, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,select_all, void, (void *box), "p", box);
WRAPPER(GtkListBoxClass,unselect_all, void, (void *box), "p", box);

#define SUPERGO()                  \
    GO(row_selected, vFpp);        \
    GO(row_activated, vFpp);    \
    GO(activate_cursor_row, vFp);    \
    GO(toggle_cursor_row, vFp);    \
    GO(move_cursor, vFpii);    \
    GO(selected_rows_changed, vFp);    \
    GO(select_all, vFp);    \
    GO(unselect_all, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkListBoxClass(my_GtkListBoxClass_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkListBoxInstance(my_GtkListBox_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkListBoxInstance(my_GtkListBox_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkListBoxRowClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkListBoxRowClass, activate, void, (void *row), "p", row);

#define SUPERGO()       \
    GO(activate, vFpp);

// wrap (so bridge all calls, just in case)
static void wrapGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkListBoxRowClass(my_GtkListBoxRowClass_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkListBoxRowClass (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkListBoxRowInstance(my_GtkListBoxRow_t* class)
{
    unwrapGtkBin3Instance(&class->parent);
}
// autobridge
static void bridgeGtkListBoxRowInstance(my_GtkListBoxRow_t* class)
{
    bridgeGtkBin3Instance(&class->parent);
}

// ----- GtkTable2Class ------
// wrap (so bridge all calls, just in case)
static void wrapGtkTable2Class(my_GtkTable2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTable2Class(my_GtkTable2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkTable2Class(my_GtkTable2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

static void unwrapGtkTable2Instance(my_GtkTable2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
static void bridgeGtkTable2Instance(my_GtkTable2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}
// ----- GtkFixed2Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
}
// autobridge
static void bridgeGtkFixed2Class(my_GtkFixed2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
}

static void unwrapGtkFixed2Instance(my_GtkFixed2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkFixed2Instance(my_GtkFixed2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkFixed3Class ------

// wrap (so bridge all calls, just in case)
static void wrapGtkFixed3Class(my_GtkFixed3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFixed3Class(my_GtkFixed3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
}
// autobridge
static void bridgeGtkFixed3Class(my_GtkFixed3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
}

static void unwrapGtkFixed3Instance(my_GtkFixed3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkFixed3Instance(my_GtkFixed3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- MetaFrames2Class ------

// wrap (so bridge all calls, just in case)
static void wrapMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    wrapGtkWindow2Class(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    unwrapGtkWindow2Class(&class->parent_class);
}
// autobridge
static void bridgeMetaFrames2Class(my_MetaFrames2Class_t* class)
{
    bridgeGtkWindow2Class(&class->parent_class);
}

static void unwrapMetaFrames2Instance(my_MetaFrames2_t* class)
{
    unwrapGtkWindow2Instance(&class->parent);
}
// autobridge
static void bridgeMetaFrames2Instance(my_MetaFrames2_t* class)
{
    bridgeGtkWindow2Instance(&class->parent);
}
// ----- GtkNotebook2Class ------
WRAPPER(GtkNotebook2Class, switch_page, void, (void* notebook, void* page, uint32_t page_num), "ppp", notebook, page, page_num);
WRAPPER(GtkNotebook2Class, select_page, int, (void* notebook, int move_focus), "pi", notebook, move_focus);
WRAPPER(GtkNotebook2Class, focus_tab, int, (void* notebook, int type), "pi", notebook, type);
WRAPPER(GtkNotebook2Class, change_current_page, int, (void* notebook, int offset), "pi", notebook, offset);
WRAPPER(GtkNotebook2Class, move_focus_out,void , (void* notebook, int direction), "pi", notebook, direction);
WRAPPER(GtkNotebook2Class, reorder_tab, int, (void* notebook, int direction, int move_to_last), "pii", notebook, direction, move_to_last);
WRAPPER(GtkNotebook2Class, insert_page, int, (void* notebook, void* child, void* tab_label, void* menu_label, int position), "ppppi", notebook, child, tab_label, menu_label, position);
WRAPPER(GtkNotebook2Class, create_window, void*, (void* notebook, void* page, int x, int y), "ppii", notebook, page, x, y);

#define SUPERGO()                   \
    GO(switch_page, vFppp);         \
    GO(select_page, iFpi);          \
    GO(focus_tab, iFpi);            \
    GO(change_current_page, iFpi);  \
    GO(move_focus_out, vFpi);       \
    GO(reorder_tab, iFpii);         \
    GO(insert_page, iFppppi);       \
    GO(create_window, pFppii);      \

// wrap (so bridge all calls, just in case)
static void wrapGtkNotebook2Class(my_GtkNotebook2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkNotebook2Class(my_GtkNotebook2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkNotebook2Class(my_GtkNotebook2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkNotebook2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkNotebook2Instance(my_GtkNotebook2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkNotebook2Instance(my_GtkNotebook2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

