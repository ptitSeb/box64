// ----- GtkCellRenderer2Class ------
WRAPPER(GtkCellRenderer2Class, get_size, void, (void* cell, void* widget, void* cell_area, int* x_offset, int* y_offset, int* width, int* height), "ppppppp", cell, widget, cell_area, x_offset, y_offset, width, height);
WRAPPER(GtkCellRenderer2Class, render, void, (void* cell, void* window, void* widget, void* background_area, void* cell_area, void* expose_area, int flags), "ppppppi", cell, window, widget, background_area, cell_area, expose_area, flags);
WRAPPER(GtkCellRenderer2Class, activate, int, (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags), "ppppppi", cell, event, widget, path, background_area, cell_area, flags);
WRAPPER(GtkCellRenderer2Class, start_editing, void*, (void* cell, void* event, void* widget, void* path, void* background_area, void* cell_area, int flags), "ppppppi", cell, event, widget, path, background_area, cell_area, flags);
WRAPPER(GtkCellRenderer2Class, editing_canceled, void, (void* cell), "p", cell);
WRAPPER(GtkCellRenderer2Class, editing_started, void, (void* cell, void* editable, void* path), "ppp", cell, editable, path);
  
#define SUPERGO()                   \
    GO(get_size, vFppppppp);        \
    GO(render, vFppppppi);          \
    GO(activate, iFppppppi);        \
    GO(start_editing, pFppppppi);   \
    GO(editing_canceled, vFp);      \
    GO(editing_started, vFppp);     \

// wrap (so bridge all calls, just in case)
static void wrapGtkCellRenderer2Class(my_GtkCellRenderer2Class_t* class)
{
    wrapGtkObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCellRenderer2Class(my_GtkCellRenderer2Class_t* class)
{
    unwrapGtkObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCellRenderer2Class(my_GtkCellRenderer2Class_t* class)
{
    bridgeGtkObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCellRenderer2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class)
{
    unwrapGtkObjectInstance(&class->parent);
}
// autobridge
static void bridgeGtkCellRenderer2Instance(my_GtkCellRenderer2_t* class)
{
    bridgeGtkObjectInstance(&class->parent);
}

// ----- GtkCellRendererText2Class ------
WRAPPER(GtkCellRendererText2Class, edited, void, (void* cell_renderer_text, void* path, void* new_text), "ppp", cell_renderer_text, path, new_text);
  
#define SUPERGO()                   \
    GO(edited, vFppp);              \

// wrap (so bridge all calls, just in case)
static void wrapGtkCellRendererText2Class(my_GtkCellRendererText2Class_t* class)
{
    wrapGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCellRendererText2Class(my_GtkCellRendererText2Class_t* class)
{
    unwrapGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCellRendererText2Class(my_GtkCellRendererText2Class_t* class)
{
    bridgeGtkCellRenderer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCellRendererText2Class (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class)
{
    unwrapGtkCellRenderer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkCellRendererText2Instance(my_GtkCellRendererText2_t* class)
{
    bridgeGtkCellRenderer2Instance(&class->parent);
}

// ----- GDBusObjectManagerClientClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusObjectManagerClient,interface_proxy_signal, void, (void* manager, void* object_proxy, void* interface_proxy, void* sender_name, void* signal_name, void* parameters), "pppppp", manager, object_proxy, interface_proxy, sender_name, signal_name, parameters);
WRAPPER(GDBusObjectManagerClient,interface_proxy_properties_changed, void, (void* manager, void* object_proxy, void* interface_proxy, void* changed_properties, void* invalidated_properties), "ppppp", manager, object_proxy, interface_proxy, changed_properties, invalidated_properties);

#define SUPERGO()                                       \
    GO(interface_proxy_signal, vFpppppp);               \
    GO(interface_proxy_properties_changed, vFppppp);    \


// wrap (so bridge all calls, just in case)
static void wrapGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusObjectManagerClientClass(my_GDBusObjectManagerClientClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GDBusObjectManagerClient (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGDBusObjectManagerClientInstance(my_GDBusObjectManagerClient_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GDBusInterfaceSkeletonClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GDBusInterfaceSkeleton,get_info, void*, (void* interface_), "p", interface_);
WRAPPER_RET(GDBusInterfaceSkeleton,get_vtable, my_GDBusInterfaceVTable_t*, findFreeGDBusInterfaceVTable,(void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,get_properties, void*, (void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,flush, void, (void* interface_), "p", interface_);
WRAPPER(GDBusInterfaceSkeleton,g_authorize_method, int, (void* interface_, void* invocation), "pp", interface_, invocation);

#define SUPERGO()                   \
    GO(get_info, pFp);              \
    GO(get_vtable, pFp);            \
    GO(get_properties, pFp);        \
    GO(flush, vFp);                 \
    GO(g_authorize_method, iFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGDBusInterfaceSkeletonClass(my_GDBusInterfaceSkeletonClass_t* class)
{
    wrapGObjectClass(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGDBusInterfaceSkeletonClass(my_GDBusInterfaceSkeletonClass_t* class)
{
    unwrapGObjectClass(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGDBusInterfaceSkeletonClass(my_GDBusInterfaceSkeletonClass_t* class)
{
    bridgeGObjectClass(&class->parent);
    #define GO(A, W) autobridge_##A##_GDBusInterfaceSkeleton (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGDBusInterfaceSkeletonInstance(my_GDBusInterfaceSkeleton_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton2, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton2, released, void, (void* button), "p", button);
WRAPPER(GtkButton2, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton2, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton2, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton2, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
static void wrapGtkButton2Class(my_GtkButton2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkButton2Class(my_GtkButton2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkButton2Class(my_GtkButton2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkButton2Instance(my_GtkButton2_t* class)
{
    unwrapGtkBin2Instance(&class->bin);
}
// autobridge
static void bridgeGtkButton2Instance(my_GtkButton2_t* class)
{
    bridgeGtkBin2Instance(&class->bin);
}

// ----- GtkButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkButton3, pressed, void,  (void* button), "p", button);
WRAPPER(GtkButton3, released, void, (void* button), "p", button);
WRAPPER(GtkButton3, clicked, void,  (void* button), "p", button);
WRAPPER(GtkButton3, enter, void,    (void* button), "p", button);
WRAPPER(GtkButton3, leave, void,    (void* button), "p", button);
WRAPPER(GtkButton3, activate, void, (void* button), "p", button);

#define SUPERGO()               \
    GO(pressed, vFp);           \
    GO(released, vFp);          \
    GO(clicked, vFp);           \
    GO(enter, vFp);             \
    GO(leave, vFp);             \
    GO(activate, vFp);          \


// wrap (so bridge all calls, just in case)
static void wrapGtkButton3Class(my_GtkButton3Class_t* class)
{
    wrapGtkBin3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkButton3Class(my_GtkButton3Class_t* class)
{
    unwrapGtkBin3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkButton3Class(my_GtkButton3Class_t* class)
{
    bridgeGtkBin3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkButton3Instance(my_GtkButton3_t* class)
{
    unwrapGtkBin3Instance(&class->bin);
}
// autobridge
static void bridgeGtkButton3Instance(my_GtkButton3_t* class)
{
    bridgeGtkBin3Instance(&class->bin);
}

// ----- GtkComboBox2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkComboBox2, changed, void, (void* combo_box), "p", combo_box);
WRAPPER(GtkComboBox2, get_active_text, void*, (void* combo_box), "p", combo_box);

#define SUPERGO()               \
    GO(changed, vFp);           \
    GO(get_active_text, pFp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkComboBox2Class(my_GtkComboBox2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkComboBox2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkComboBox2Instance(my_GtkComboBox2_t* class)
{
    unwrapGtkBin2Instance(&class->parent);
}
// autobridge
static void bridgeGtkComboBox2Instance(my_GtkComboBox2_t* class)
{
    bridgeGtkBin2Instance(&class->parent);
}

// ----- GtkToggleButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton2, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
static void wrapGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    wrapGtkButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    unwrapGtkButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkToggleButton2Class(my_GtkToggleButton2Class_t* class)
{
    bridgeGtkButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkToggleButton2Instance(my_GtkToggleButton2_t* class)
{
    unwrapGtkButton2Instance(&class->button);
}
// autobridge
static void bridgeGtkToggleButton2Instance(my_GtkToggleButton2_t* class)
{
    bridgeGtkButton2Instance(&class->button);
}

// ----- GtkToggleButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkToggleButton3, toggled, void, (void* toggle_button), "p", toggle_button);

#define SUPERGO()               \
    GO(toggled, vFp);           \


// wrap (so bridge all calls, just in case)
static void wrapGtkToggleButton3Class(my_GtkToggleButton3Class_t* class)
{
    wrapGtkButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkToggleButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkToggleButton3Class(my_GtkToggleButton3Class_t* class)
{
    unwrapGtkButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkToggleButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkToggleButton3Class(my_GtkToggleButton3Class_t* class)
{
    bridgeGtkButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkToggleButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkToggleButton3Instance(my_GtkToggleButton3_t* class)
{
    unwrapGtkButton3Instance(&class->parent);
}
// autobridge
static void bridgeGtkToggleButton3Instance(my_GtkToggleButton3_t* class)
{
    bridgeGtkButton3Instance(&class->parent);
}

// ----- GtkCheckButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton2, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    wrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    unwrapGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCheckButton2Class(my_GtkCheckButton2Class_t* class)
{
    bridgeGtkToggleButton2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCheckButton2Instance(my_GtkCheckButton2_t* class)
{
    unwrapGtkToggleButton2Instance(&class->parent);
}
// autobridge
static void bridgeGtkCheckButton2Instance(my_GtkCheckButton2_t* class)
{
    bridgeGtkToggleButton2Instance(&class->parent);
}

// ----- GtkCheckButton3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkCheckButton3, draw_indicator, void, (void* check_button, void* area), "pp", check_button, area);

#define SUPERGO()               \
    GO(draw_indicator, vFpp);   \


// wrap (so bridge all calls, just in case)
static void wrapGtkCheckButton3Class(my_GtkCheckButton3Class_t* class)
{
    wrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkCheckButton3Class(my_GtkCheckButton3Class_t* class)
{
    unwrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkCheckButton3Class(my_GtkCheckButton3Class_t* class)
{
    bridgeGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkCheckButton3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkCheckButton3Instance(my_GtkCheckButton3_t* class)
{
    unwrapGtkToggleButton3Instance(&class->parent);
}
// autobridge
static void bridgeGtkCheckButton3Instance(my_GtkCheckButton3_t* class)
{
    bridgeGtkToggleButton3Instance(&class->parent);
}
// ----- GtkGtkMenuButton3Class ------
// wrapper x86 -> natives of callbacks

#define SUPERGO() \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuButton3Class(my_GtkMenuButton3Class_t* class)
{
    wrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuButton3Class(my_GtkMenuButton3Class_t* class)
{
    unwrapGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuButton3Class(my_GtkMenuButton3Class_t* class)
{
    bridgeGtkToggleButton3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuButton3 (W, class->A)
    SUPERGO()
    #undef GO
}
#undef SUPERGO

static void unwrapGtkMenuButton3Instance(my_GtkMenuButton3_t* class)
{
    unwrapGtkToggleButton3Instance(&class->parent);
}
// autobridge
static void bridgeGtkMenuButton3Instance(my_GtkMenuButton3_t* class)
{
    bridgeGtkToggleButton3Instance(&class->parent);
}
// ----- GtkEntry2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEntry2, populate_popup, void,     (void* entry, void* menu), "pp", entry, menu);
WRAPPER(GtkEntry2, activate, void,           (void* entry), "p", entry);
WRAPPER(GtkEntry2, move_cursor, void,        (void* entry, int step, int count, int extend_selection), "piii", entry, step, count, extend_selection);
WRAPPER(GtkEntry2, insert_at_cursor, void,   (void* entry, void* str), "pp", entry, str);
WRAPPER(GtkEntry2, delete_from_cursor, void, (void* entry, size_t type, int count), "pLi", entry, type, count);
WRAPPER(GtkEntry2, backspace, void,          (void* entry), "p", entry);
WRAPPER(GtkEntry2, cut_clipboard, void,      (void* entry), "p", entry);
WRAPPER(GtkEntry2, copy_clipboard, void,     (void* entry), "p", entry);
WRAPPER(GtkEntry2, paste_clipboard, void,    (void* entry), "p", entry);
WRAPPER(GtkEntry2, toggle_overwrite, void,   (void* entry), "p", entry);
WRAPPER(GtkEntry2, get_text_area_size, void, (void* entry, void* x, void* y, void* width, void* height), "ppppp", entry, x, y, width, height);

#define SUPERGO()                   \
    GO(populate_popup, vFpp);       \
    GO(activate, vFp);              \
    GO(move_cursor, vFpiii);        \
    GO(insert_at_cursor, vFp);      \
    GO(delete_from_cursor, vFpii);  \
    GO(backspace, vFp);             \
    GO(cut_clipboard, vFp);         \
    GO(copy_clipboard, vFp);        \
    GO(paste_clipboard, vFp);       \
    GO(toggle_overwrite, vFp);      \
    GO(get_text_area_size, vFppppp);\

// wrap (so bridge all calls, just in case)
static void wrapGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkEntry2Class(my_GtkEntry2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEntry2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkEntry2Instance(my_GtkEntry2_t* class)
{
    unwrapGtkWidget2Instance(&class->parent);
}
// autobridge
static void bridgeGtkEntry2Instance(my_GtkEntry2_t* class)
{
    bridgeGtkWidget2Instance(&class->parent);
}

// ----- GtkSpinButton2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkSpinButton2, input, int,  (void* spin_button, void* new_value), "pp", spin_button, new_value);
WRAPPER(GtkSpinButton2, output, int, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, value_changed, void, (void* spin_button), "p", spin_button);
WRAPPER(GtkSpinButton2, change_value, void, (void* spin_button, int scroll), "pi", spin_button, scroll);
WRAPPER(GtkSpinButton2, wrapped, void, (void* spin_button), "p", spin_button);

#define SUPERGO()           \
    GO(input, iFpp);        \
    GO(output, iFp);        \
    GO(value_changed, vFp); \
    GO(change_value, vFpi); \
    GO(wrapped, vFp);       \

// wrap (so bridge all calls, just in case)
static void wrapGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    wrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    unwrapGtkEntry2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkSpinButton2Class(my_GtkSpinButton2Class_t* class)
{
    bridgeGtkEntry2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkSpinButton2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkSpinButton2Instance(my_GtkSpinButton2_t* class)
{
    unwrapGtkEntry2Instance(&class->entry);
}
// autobridge
static void bridgeGtkSpinButton2Instance(my_GtkSpinButton2_t* class)
{
    bridgeGtkEntry2Instance(&class->entry);
}

// ----- GtkProgress2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkProgress2, paint, void,          (void* progress), "p", progress);
WRAPPER(GtkProgress2, update, void,         (void* progress), "p", progress);
WRAPPER(GtkProgress2, act_mode_enter, void, (void* progress), "p", progress);

#define SUPERGO()           \
    GO(paint, vFp);         \
    GO(update, vFp);        \
    GO(act_mode_enter, vFp);\

// wrap (so bridge all calls, just in case)
static void wrapGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    wrapGtkWidget2Class(&class->parent);
    #define GO(A, W) class->A = reverse_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    unwrapGtkWidget2Class(&class->parent);
    #define GO(A, W)   class->A = find_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkProgress2Class(my_GtkProgress2Class_t* class)
{
    bridgeGtkWidget2Class(&class->parent);
    #define GO(A, W) autobridge_##A##_GtkProgress2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkProgress2Instance(my_GtkProgress2_t* class)
{
    unwrapGtkWidget2Instance(&class->widget);
}
// autobridge
static void bridgeGtkProgress2Instance(my_GtkProgress2_t* class)
{
    bridgeGtkWidget2Instance(&class->widget);
}

// ----- GtkProgressBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    wrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    unwrapGtkProgress2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkProgressBar2Class(my_GtkProgressBar2Class_t* class)
{
    bridgeGtkProgress2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkProgressBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkProgressBar2Instance(my_GtkProgressBar2_t* class)
{
    unwrapGtkProgress2Instance(&class->parent);
}
// autobridge
static void bridgeGtkProgressBar2Instance(my_GtkProgressBar2_t* class)
{
    bridgeGtkProgress2Instance(&class->parent);
}

// ----- GtkFrame2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkFrame2, compute_child_allocation, void, (void* frame, void* allocation), "pp", frame, allocation);

#define SUPERGO()                       \
    GO(compute_child_allocation, vFpp); \

// wrap (so bridge all calls, just in case)
static void wrapGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    wrapGtkBin2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    unwrapGtkBin2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkFrame2Class(my_GtkFrame2Class_t* class)
{
    bridgeGtkBin2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkFrame2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkFrame2Instance(my_GtkFrame2_t* class)
{
    unwrapGtkBin2Instance(&class->bin);
}
// autobridge
static void bridgeGtkFrame2Instance(my_GtkFrame2_t* class)
{
    bridgeGtkBin2Instance(&class->bin);
}

// ----- GtkMenuShell2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkMenuShell2,deactivate, void,      (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,selection_done, void,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_current, void,    (void* menu_shell, int direction),  "pi", menu_shell, direction);
WRAPPER(GtkMenuShell2,activate_current, void,(void* menu_shell, int force_hide), "pi", menu_shell, force_hide);
WRAPPER(GtkMenuShell2,cancel, void,          (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,select_item, void,     (void* menu_shell, void* menu_item), "pp", menu_shell, menu_item);
WRAPPER(GtkMenuShell2,insert, void,          (void* menu_shell, void* child, int position), "ppi", menu_shell, child, position);
WRAPPER(GtkMenuShell2,get_popup_delay, int,  (void* menu_shell), "p", menu_shell);
WRAPPER(GtkMenuShell2,move_selected, int,    (void* menu_shell, int distance), "pi", menu_shell, distance);

#define SUPERGO()               \
    GO(deactivate, vFp);        \
    GO(selection_done, vFp);    \
    GO(move_current, vFpi);     \
    GO(activate_current, vFpi); \
    GO(cancel, vFp);            \
    GO(select_item, vFpp);      \
    GO(insert, vFppi);          \
    GO(get_popup_delay, iFp);   \
    GO(move_selected, iFpi);    \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuShell2Class(my_GtkMenuShell2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuShell2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkMenuShell2Instance(my_GtkMenuShell2_t* class)
{
    unwrapGtkContainer2Instance(&class->container);
}
// autobridge
static void bridgeGtkMenuShell2Instance(my_GtkMenuShell2_t* class)
{
    bridgeGtkContainer2Instance(&class->container);
}

// ----- GtkMenuBar2Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    wrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    unwrapGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkMenuBar2Class(my_GtkMenuBar2Class_t* class)
{
    bridgeGtkMenuShell2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkMenuBar2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkMenuBar2Instance(my_GtkMenuBar2_t* class)
{
    unwrapGtkMenuShell2Instance(&class->parent);
}
// autobridge
static void bridgeGtkMenuBar2Instance(my_GtkMenuBar2_t* class)
{
    bridgeGtkMenuShell2Instance(&class->parent);
}

// ----- GtkTextView2Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView2, set_scroll_adjustments, void,   (void* text_view, void* hadjustment, void* vadjustment), "ppp", text_view, hadjustment, vadjustment);
WRAPPER(GtkTextView2, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView2, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView2, page_horizontally, void,        (void* text_view, int count, int extend_selection), "pii", text_view, count, extend_selection);
WRAPPER(GtkTextView2, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView2, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView2, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView2, move_focus, void,               (void* text_view, int direction), "pi", text_view, direction);

#define SUPERGO()                       \
    GO(set_scroll_adjustments, vFppp);  \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(page_horizontally, vFpii);       \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(move_focus, vFpi);               \

// wrap (so bridge all calls, just in case)
static void wrapGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    wrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    unwrapGtkContainer2Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTextView2Class(my_GtkTextView2Class_t* class)
{
    bridgeGtkContainer2Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView2 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkTextView2Instance(my_GtkTextView2_t* class)
{
    unwrapGtkContainer2Instance(&class->parent);
}
// autobridge
static void bridgeGtkTextView2Instance(my_GtkTextView2_t* class)
{
    bridgeGtkContainer2Instance(&class->parent);
}

// ----- GtkTextView3Class ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkTextView3, populate_popup, void,           (void* text_view, void* menu), "pp", text_view, menu);
WRAPPER(GtkTextView3, move_cursor, void,              (void* text_view, int step, int count, int extend_selection), "piii", text_view, step, count, extend_selection);
WRAPPER(GtkTextView3, set_anchor, void,               (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, insert_at_cursor, void,         (void* text_view, void* str), "pp", text_view, str);
WRAPPER(GtkTextView3, delete_from_cursor, void,       (void* text_view, int type, int count), "pii", text_view, type, count);
WRAPPER(GtkTextView3, backspace, void,                (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, cut_clipboard, void,            (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, copy_clipboard, void,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, paste_clipboard, void,          (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, toggle_overwrite, void,         (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, create_buffer, void*,           (void* text_view), "p", text_view);
WRAPPER(GtkTextView3, draw_layer, void,               (void* text_view, int layer, void* cr), "pip", text_view, layer, cr);
WRAPPER(GtkTextView3, extend_selection, int,          (void* text_view, int granularity, void* location, void* start, void* end), "pippp", text_view, granularity, location, start, end);
WRAPPER(GtkTextView3, insert_emoji, void,             (void* text_view), "p", text_view);

#define SUPERGO()                       \
    GO(populate_popup, vFpp);           \
    GO(move_cursor, vFpiii);            \
    GO(set_anchor, vFp);                \
    GO(insert_at_cursor, vFpp);         \
    GO(delete_from_cursor, vFpii);      \
    GO(backspace, vFp);                 \
    GO(cut_clipboard, vFp);             \
    GO(copy_clipboard, vFp);            \
    GO(paste_clipboard, vFp);           \
    GO(toggle_overwrite, vFp);          \
    GO(create_buffer, pFp);             \
    GO(draw_layer, vFpip);              \
    GO(extend_selection, iFpippp);      \
    GO(insert_emoji, vFp);              \

// wrap (so bridge all calls, just in case)
static void wrapGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkTextView3Class(my_GtkTextView3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkTextView3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkTextView3Instance(my_GtkTextView3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkTextView3Instance(my_GtkTextView3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

// ----- GtkGrid3Class ------
// no wrapper x86 -> natives of callbacks

#define SUPERGO()           \

// wrap (so bridge all calls, just in case)
static void wrapGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    wrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    unwrapGtkContainer3Class(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGrid3Class(my_GtkGrid3Class_t* class)
{
    bridgeGtkContainer3Class(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGrid3 (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkGrid3Instance(my_GtkGrid3_t* class)
{
    unwrapGtkContainer3Instance(&class->parent);
}
// autobridge
static void bridgeGtkGrid3Instance(my_GtkGrid3_t* class)
{
    bridgeGtkContainer3Instance(&class->parent);
}

