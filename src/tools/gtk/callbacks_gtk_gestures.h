// ----- GtkEventControllerClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkEventController, set_widget, void, (void* controller, void* widget), "pp", controller, widget);
WRAPPER(GtkEventController, unset_widget, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_event, int, (void *controller, void *event, double x, double y), "ppdd", controller, event, x, y);
WRAPPER(GtkEventController, reset, void, (void* controller), "p", controller);
WRAPPER(GtkEventController, handle_crossing, void, (void *controller, void *crossing, double x, double y), "ppdd", controller, crossing, x, y);
WRAPPER(GtkEventController, filter_event, void, (void *controller, void *event), "pp", controller, event);

#define SUPERGO()               \
    GO(set_widget, vFpp);       \
    GO(unset_widget, vFp);      \
    GO(handle_event, iFppdd);   \
    GO(reset, vFp);             \
    GO(handle_crossing, vFppdd);\
    GO(filter_event, vFpp);

// wrap (so bridge all calls, just in case)
static void wrapGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    wrapGObjectClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkEventController(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    unwrapGObjectClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkEventControllerClass(my_GtkEventControllerClass_t* class)
{
    bridgeGObjectClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkEventController (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkEventControllerInstance(my_GtkEventController_t* class)
{
    unwrapGObjectInstance(&class->parent);
}
// autobridge
static void bridgeGtkEventControllerInstance(my_GtkEventController_t* class)
{
    bridgeGObjectInstance(&class->parent);
}

// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGesture, check, void, (void* gesture), "p", gesture);
WRAPPER(GtkGesture, begin, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, update, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, end, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, cancel, void, (void *gesture, void *sequence), "pp", gesture, sequence);
WRAPPER(GtkGesture, sequence_state_changed, void, (void *gesture, void *sequence, int state), "ppi", gesture, sequence, state);

#define SUPERGO()              \
    GO(check, vFp);            \
    GO(begin, vFpp);           \
    GO(update, vFpp);          \
    GO(end, vFpp);             \
    GO(cancel, vFpp);          \
    GO(sequence_state_changed, vFppi);

// wrap (so bridge all calls, just in case)
static void wrapGtkGestureClass(my_GtkGestureClass_t* class)
{
    wrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGesture(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureClass(my_GtkGestureClass_t* class)
{
    unwrapGtkEventControllerClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGestureClass(my_GtkGestureClass_t* class)
{
    bridgeGtkEventControllerClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGesture (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkGestureInstance(my_GtkGesture_t* class)
{
    unwrapGtkEventControllerInstance(&class->parent);
}
// autobridge
static void bridgeGtkGestureInstance(my_GtkGesture_t* class)
{
    bridgeGtkEventControllerInstance(&class->parent);
}

// ----- GtkGestureSingleClass ------
// wrap (so bridge all calls, just in case)
static void wrapGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    wrapGtkGestureClass(&class->parent_class);
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    unwrapGtkGestureClass(&class->parent_class);
}
// autobridge
static void bridgeGtkGestureSingleClass(my_GtkGestureSingleClass_t* class)
{
    bridgeGtkGestureClass(&class->parent_class);
}

static void unwrapGtkGestureSingleInstance(my_GtkGestureSingle_t* class)
{
    unwrapGtkGestureInstance(&class->parent);
}
// autobridge
static void bridgeGtkGestureSingleInstance(my_GtkGestureSingle_t* class)
{
    bridgeGtkGestureInstance(&class->parent);
}
// ----- GtkGestureClass ------
// wrapper x86 -> natives of callbacks
WRAPPER(GtkGestureLongPress, pressed, void, (void *gesture, double x, double y), "pdd", gesture, x, y);
WRAPPER(GtkGestureLongPress, cancelled, void, (void *cancelled), "p", cancelled);

#define SUPERGO()              \
    GO(pressed, vFpdd);          \
    GO(cancelled, vFp);

// wrap (so bridge all calls, just in case)
static void wrapGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    wrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) class->A = reverse_##A##_GtkGestureLongPress(W, class->A)
    SUPERGO()
    #undef GO
}
// unwrap (and use callback if not a native call anymore)
static void unwrapGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    unwrapGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W)   class->A = find_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}
// autobridge
static void bridgeGtkGestureLongPressClass(my_GtkGestureLongPressClass_t* class)
{
    bridgeGtkGestureSingleClass(&class->parent_class);
    #define GO(A, W) autobridge_##A##_GtkGestureLongPress (W, class->A)
    SUPERGO()
    #undef GO
}

#undef SUPERGO

static void unwrapGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class)
{
    unwrapGtkGestureSingleInstance(&class->parent);
}
// autobridge
static void bridgeGtkGestureLongPressInstance(my_GtkGestureLongPress_t* class)
{
    bridgeGtkGestureSingleInstance(&class->parent);
}

