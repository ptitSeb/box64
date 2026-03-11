// a class to collection of custom defined class...
void addRegisteredClass(size_t klass, char* name)
{
    if(!klass)
        return;
    if(!my_customclass) {
        my_customclass = kh_init(customclass);
    }
    khint_t k;
    int ret;
    k = kh_put(customclass, my_customclass, klass, &ret);
    kh_value(my_customclass, k) = strdup(name);
}

int checkRegisteredClass(size_t klass)
{
    if(!my_customclass)
        return 0;
    khint_t k = kh_get(customclass, my_customclass, klass);
    return (k==kh_end(my_customclass))?0:1;
}

// g_type_class_peek_parent
void wrapGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        wrap##A##Class((my_##A##Class_t*)cl);       \
    else

    printf_log(LOG_DEBUG, "wrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        if(my_MetaFrames2==(size_t)-1 && !strcmp(g_type_name(type), "MetaFrames")) {
            my_MetaFrames2 = type;
            wrapMetaFrames2Class((my_MetaFrames2Class_t*)cl);
        } else
            printf_log(LOG_NONE, "Warning, Custom Class initializer with unknown class type 0w%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void unwrapGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        unwrap##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "...unwrapGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else
        printf_log(LOG_NONE, "Warning: fail to unwrapGTKClass for type %zx (%s)\n", type, g_type_name(type));
    #undef GTKCLASS
    #undef GTKIFACE
}

static void bridgeGTKClass(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        bridge##A##Class((my_##A##Class_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "bridgeGTKClass(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

static void wrapGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        wrap##A##Interface((my_##A##Interface_t*)cl);   \
    else

    printf_log(LOG_DEBUG, "wrapGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, Custom Interface initializer with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKIFACE
    #undef GTKCLASS
}

void unwrapGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        unwrap##A##Interface((my_##A##Interface_t*)cl); \
    else

    printf_log(LOG_DEBUG, "unwrapGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else
    {}  // else no warning, one is enough...
    #undef GTKIFACE
    #undef GTKCLASS
}

static void bridgeGTKInterface(void* cl, size_t type)
{
    #define GTKCLASS(A)
    #define GTKIFACE(A)                                 \
    if(type==my_##A)                                    \
        bridge##A##Interface((my_##A##Interface_t*)cl); \
    else

    printf_log(LOG_DEBUG, "bridgeGTKInterface(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Interface with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

void unwrapGTKInstance(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        unwrap##A##Instance((my_##A##_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "unwrapGTKInstance(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else
    {}  // else no warning, one is enough...
    #undef GTKCLASS
    #undef GTKIFACE
}

void bridgeGTKInstance(void* cl, size_t type)
{
    #define GTKIFACE(A)
    #define GTKCLASS(A)                             \
    if(type==my_##A)                                \
        bridge##A##Instance((my_##A##_t*)cl);     \
    else

    printf_log(LOG_DEBUG, "bridgeGTKInstance(%p, %zd (%s))\n", cl, type, g_type_name(type));
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, AutoBridge GTK Class with unknown class type 0w%zx (%s)\n", type, g_type_name(type));
    }
    #undef GTKCLASS
    #undef GTKIFACE
}

typedef union my_GClassAll_s {
    #define GTKCLASS(A) my_##A##Class_t A;
    #define GTKIFACE(A) my_##A##Interface_t A;
    GTKCLASSES()
    #undef GTKIFACE
    #undef GTKCLASS
} my_GClassAll_t;

#define GO(A) \
static void* my_gclassall_ref_##A = NULL;   \
static my_GClassAll_t my_gclassall_##A;

SUPER()
#undef GO

void* unwrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    if(checkRegisteredClass(type))
        return klass;
    #define GO(A) if(klass == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKIFACE(A)
    #define GTKCLASS(A) if(type==my_##A) sz = sizeof(my_##A##Class_t); else
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKClass called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
        return klass;
    }
    #undef GTKCLASS
    #undef GTKIFACE
    my_GClassAll_t *newklass = NULL;
    #define GO(A) if(!newklass && !my_gclassall_ref_##A) {my_gclassall_ref_##A = klass; newklass = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newklass) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKClass\n");
        return klass;
    }
    memcpy(newklass, klass, sz);
    unwrapGTKClass(newklass, type);
    return newklass;
}

void* unwrapCopyGTKInterface(void* iface, size_t type)
{
    if(!iface) return iface;
    if(checkRegisteredClass(type))
        return iface;
    #define GO(A) if(iface == my_gclassall_ref_##A) return &my_gclassall_##A;
    SUPER()
    #undef GO
    // check if class is the exact type we know
    size_t sz = 0;
    #define GTKIFACE(A) if(type==my_##A) sz = sizeof(my_##A##Interface_t); else
    #define GTKCLASS(A)
    GTKCLASSES()
    if(type<0x35) {}  // GInterface (8) and other simple opbjects have no structure
    else {
        printf_log(LOG_NONE, "Warning, unwrapCopyGTKInterface called with unknown class type 0x%zx (%s)\n", type, g_type_name(type));
        return iface;
    }
    #undef GTKCLASS
    #undef GTKIFACE
    my_GClassAll_t *newiface = NULL;
    #define GO(A) if(!newiface && !my_gclassall_ref_##A) {my_gclassall_ref_##A = iface; newiface = &my_gclassall_##A;}
    SUPER()
    #undef GO
    if(!newiface) {
        printf_log(LOG_NONE, "Warning: no more slot for unwrapCopyGTKInterface\n");
        return iface;
    }
    memcpy(newiface, iface, sz);
    unwrapGTKInterface(newiface, type);
    return newiface;
}

// gtk_type_class

void* wrapCopyGTKClass(void* klass, size_t type)
{
    if(!klass) return klass;
    while(checkRegisteredClass(type))
        type = g_type_parent(type);
    printf_log(LOG_DEBUG, "wrapCopyGTKClass(%p, %zd (%s))\n", klass, type, g_type_name(type));
    bridgeGTKClass(klass, type);
    return klass;
}

void* wrapCopyGTKInterface(void* iface, size_t type)
{
    if(!iface) return iface;
    while(checkRegisteredClass(type))
        type = g_type_parent(type);
    printf_log(LOG_DEBUG, "wrapCopyGTKInterface(%p, %zd (%s))\n", iface, type, g_type_name(type));
    bridgeGTKInterface(iface, type);
    return iface;
}
// ---- GTypeValueTable ----

// First the structure GTypeInfo statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeValueTable_t     my_gtypevaluetable_##A = {0};   \
static my_GTypeValueTable_t   *ref_gtypevaluetable_##A = NULL;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
// value_init ...
#define GO(A)   \
static uintptr_t my_value_init_fct_##A = 0;                     \
static void my_value_init_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_value_init_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_value_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_init_fct_##A == (uintptr_t)fct) return my_value_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_init_fct_##A == 0) {my_value_init_fct_##A = (uintptr_t)fct; return my_value_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_init callback\n");
    return NULL;
}
// value_free ...
#define GO(A)   \
static uintptr_t my_value_free_fct_##A = 0;                     \
static void my_value_free_##A(void* a)                          \
{                                                               \
    RunFunctionFmt(my_value_free_fct_##A, "p", a);  \
}
SUPER()
#undef GO
static void* find_value_free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_free_fct_##A == (uintptr_t)fct) return my_value_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_free_fct_##A == 0) {my_value_free_fct_##A = (uintptr_t)fct; return my_value_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_free callback\n");
    return NULL;
}
// value_copy ...
#define GO(A)   \
static uintptr_t my_value_copy_fct_##A = 0;                     \
static void my_value_copy_##A(void* a, void* b)                 \
{                                                               \
    RunFunctionFmt(my_value_copy_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_value_copy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_copy_fct_##A == (uintptr_t)fct) return my_value_copy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_copy_fct_##A == 0) {my_value_copy_fct_##A = (uintptr_t)fct; return my_value_copy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_copy callback\n");
    return NULL;
}
// value_peek_pointer ...
#define GO(A)   \
static uintptr_t my_value_peek_pointer_fct_##A = 0;                                 \
static void* my_value_peek_pointer_##A(void* a)                                     \
{                                                                                   \
    return (void*)RunFunctionFmt(my_value_peek_pointer_fct_##A, "p", a);\
}
SUPER()
#undef GO
static void* find_value_peek_pointer_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_value_peek_pointer_fct_##A == (uintptr_t)fct) return my_value_peek_pointer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_value_peek_pointer_fct_##A == 0) {my_value_peek_pointer_fct_##A = (uintptr_t)fct; return my_value_peek_pointer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo value_peek_pointer callback\n");
    return NULL;
}
// collect_value ...
#define GO(A)   \
static uintptr_t my_collect_value_fct_##A = 0;                                              \
static void* my_collect_value_##A(void* a, uint32_t b, void* c, uint32_t d)                 \
{                                                                                           \
    return (void*)RunFunctionFmt(my_collect_value_fct_##A, "pupu", a, b, c, d); \
}
SUPER()
#undef GO
static void* find_collect_value_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_collect_value_fct_##A == (uintptr_t)fct) return my_collect_value_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_collect_value_fct_##A == 0) {my_collect_value_fct_##A = (uintptr_t)fct; return my_collect_value_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo collect_value callback\n");
    return NULL;
}
// lcopy_value ...
#define GO(A)   \
static uintptr_t my_lcopy_value_fct_##A = 0;                                                \
static void* my_lcopy_value_##A(void* a, uint32_t b, void* c, uint32_t d)                   \
{                                                                                           \
    return (void*)RunFunctionFmt(my_lcopy_value_fct_##A, "pupu", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_lcopy_value_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_lcopy_value_fct_##A == (uintptr_t)fct) return my_lcopy_value_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lcopy_value_fct_##A == 0) {my_lcopy_value_fct_##A = (uintptr_t)fct; return my_lcopy_value_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo lcopy_value callback\n");
    return NULL;
}
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeValueTable_t* findFreeGTypeValueTable(my_GTypeValueTable_t* fcts)
{
    if(!fcts) return fcts;
    #define GO(A) if(ref_gtypevaluetable_##A == fcts) return &my_gtypevaluetable_##A;
    SUPER()
    #undef GO
    #define GO(A) if(ref_gtypevaluetable_##A == 0) {                                                        \
        ref_gtypevaluetable_##A = fcts;                                                                     \
        my_gtypevaluetable_##A.value_init = find_value_init_Fct(fcts->value_init);                          \
        my_gtypevaluetable_##A.value_free = find_value_free_Fct(fcts->value_free);                          \
        my_gtypevaluetable_##A.value_copy = find_value_copy_Fct(fcts->value_copy);                          \
        my_gtypevaluetable_##A.value_peek_pointer = find_value_peek_pointer_Fct(fcts->value_peek_pointer);  \
        my_gtypevaluetable_##A.collect_format = fcts->collect_format;                                       \
        my_gtypevaluetable_##A.collect_value = find_collect_value_Fct(fcts->collect_value);                 \
        my_gtypevaluetable_##A.lcopy_format = fcts->lcopy_format;                                           \
        my_gtypevaluetable_##A.lcopy_value = find_lcopy_value_Fct(fcts->lcopy_value);                       \
        return &my_gtypevaluetable_##A;                                                                     \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeValueTable callback\n");
    return NULL;
}

// signal2 ...
#define GO(A)   \
static uintptr_t my_signal2_fct_##A = 0;                                        \
static void* my_signal2_##A(void* a, void* b)                                   \
{                                                                               \
    return (void*)RunFunctionFmt(my_signal2_fct_##A, "pp", a, b);   \
}
SUPER()
#undef GO
static void* find_signal2_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal2_fct_##A == (uintptr_t)fct) return my_signal2_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal2_fct_##A == 0) {my_signal2_fct_##A = (uintptr_t)fct; return my_signal2_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal2 callback\n");
    return NULL;
}
// signal3 ...
#define GO(A)   \
static uintptr_t my_signal3_fct_##A = 0;                                            \
static void* my_signal3_##A(void* a, void* b, void* c)                              \
{                                                                                   \
    return (void*)RunFunctionFmt(my_signal3_fct_##A, "ppp", a, b, c);   \
}
SUPER()
#undef GO
static void* find_signal3_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal3_fct_##A == (uintptr_t)fct) return my_signal3_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal3_fct_##A == 0) {my_signal3_fct_##A = (uintptr_t)fct; return my_signal3_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal3 callback\n");
    return NULL;
}
// signal4 ...
#define GO(A)   \
static uintptr_t my_signal4_fct_##A = 0;                                                \
static void* my_signal4_##A(void* a, void* b, void* c, void* d)                         \
{                                                                                       \
    return (void*)RunFunctionFmt(my_signal4_fct_##A, "pppp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_signal4_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal4_fct_##A == (uintptr_t)fct) return my_signal4_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal4_fct_##A == 0) {my_signal4_fct_##A = (uintptr_t)fct; return my_signal4_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal4 callback\n");
    return NULL;
}
// signal5 ...
#define GO(A)   \
static uintptr_t my_signal5_fct_##A = 0;                                                    \
static void* my_signal5_##A(void* a, void* b, void* c, void* d, void* e)                    \
{                                                                                           \
    return (void*)RunFunctionFmt(my_signal5_fct_##A, "ppppp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_signal5_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal5_fct_##A == (uintptr_t)fct) return my_signal5_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal5_fct_##A == 0) {my_signal5_fct_##A = (uintptr_t)fct; return my_signal5_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal5 callback\n");
    return NULL;
}
// signal6 ...
#define GO(A)   \
static uintptr_t my_signal6_fct_##A = 0;                                                        \
static void* my_signal6_##A(void* a, void* b, void* c, void* d, void* e, void* f)               \
{                                                                                               \
    return (void*)RunFunctionFmt(my_signal6_fct_##A, "pppppp", a, b, c, d, e, f);   \
}
SUPER()
#undef GO
static void* find_signal6_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal6_fct_##A == (uintptr_t)fct) return my_signal6_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal6_fct_##A == 0) {my_signal6_fct_##A = (uintptr_t)fct; return my_signal6_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal6 callback\n");
    return NULL;
}
// signal7 ...
#define GO(A)   \
static uintptr_t my_signal7_fct_##A = 0;                                                            \
static void* my_signal7_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g)          \
{                                                                                                   \
    return (void*)RunFunctionFmt(my_signal7_fct_##A, "ppppppp", a, b, c, d, e, f, g);   \
}
SUPER()
#undef GO
static void* find_signal7_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_signal7_fct_##A == (uintptr_t)fct) return my_signal7_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_signal7_fct_##A == 0) {my_signal7_fct_##A = (uintptr_t)fct; return my_signal7_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal7 callback\n");
    return NULL;
}
// signal8 ...
#define GO(A)                                                                                  \
    static uintptr_t my_signal8_fct_##A = 0;                                                   \
    static void* my_signal8_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                          \
        return (void*)RunFunctionFmt(my_signal8_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal8_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal8_fct_##A == (uintptr_t)fct) return my_signal8_##A;
    SUPER()
#undef GO
#define GO(A)                                \
    if (my_signal8_fct_##A == 0) {           \
        my_signal8_fct_##A = (uintptr_t)fct; \
        return my_signal8_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal8 callback\n");
    return NULL;
}
// signal9 ...
#define GO(A)                                                                                  \
    static uintptr_t my_signal9_fct_##A = 0;                                                   \
    static void* my_signal9_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                          \
        return (void*)RunFunctionFmt(my_signal9_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal9_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal9_fct_##A == (uintptr_t)fct) return my_signal9_##A;
    SUPER()
#undef GO
#define GO(A)                                \
    if (my_signal9_fct_##A == 0) {           \
        my_signal9_fct_##A = (uintptr_t)fct; \
        return my_signal9_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal9 callback\n");
    return NULL;
}
// signal10 ...
#define GO(A)                                                                                   \
    static uintptr_t my_signal10_fct_##A = 0;                                                   \
    static void* my_signal10_##A(void* a, void* b, void* c, void* d, void* e, void* f, void* g) \
    {                                                                                           \
        return (void*)RunFunctionFmt(my_signal10_fct_##A, "ppppppp", a, b, c, d, e, f, g);      \
    }
SUPER()
#undef GO
static void* find_signal10_Fct(void* fct)
{
    if (!fct) return fct;
    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);
#define GO(A) \
    if (my_signal10_fct_##A == (uintptr_t)fct) return my_signal10_##A;
    SUPER()
#undef GO
#define GO(A)                                 \
    if (my_signal10_fct_##A == 0) {           \
        my_signal10_fct_##A = (uintptr_t)fct; \
        return my_signal10_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo signal10 callback\n");
    return NULL;
}
typedef void* (*finder_t)(void*);
static const finder_t finders[] = { find_signal2_Fct, find_signal3_Fct, find_signal4_Fct, find_signal5_Fct, find_signal6_Fct, find_signal7_Fct, find_signal8_Fct, find_signal9_Fct, find_signal10_Fct };
#define MAX_SIGNAL_N (10 - 2)

// ---- GTypeInfo ----
// let's handle signal with offset, that are used to wrap custom signal function
void my_unwrap_signal_offset(void* klass);
void my_add_signal_offset(size_t itype, uint32_t offset, int n)
{
    printf_log(LOG_DEBUG, "my_add_signal_offset(0x%zx, %d, %d)\n", itype, offset, n);
    if(!offset || !itype) // no offset means no overload...
        return;
    if(n<0 || n>MAX_SIGNAL_N) {
        printf_log(LOG_NONE, "Warning, signal with too many args (%d) in my_add_signal_offset\n", n);
        return;
    }
    int ret;
    khint_t k = kh_put(sigoffset, my_sigoffset, itype, &ret);
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    if(ret) {
        p->a = NULL; p->cap = 0; p->sz = 0;
    }
    // check if offset already there
    for(int i=0; i<p->sz; ++i)
        if(p->a[i].offset == offset) {
            printf_log(LOG_INFO, "Offset already there... Bye\n");
            return; // already there, bye
        }
    if(p->sz==p->cap) {
        p->cap+=4;
        p->a = (sigoffset_t*)box_realloc(p->a, sizeof(sigoffset_t)*p->cap);
    }
    p->a[p->sz].offset = offset;
    p->a[p->sz++].n = n;
}
void my_unwrap_signal_offset(void* klass)
{
    if(!klass)
        return;
    size_t itype = *(size_t*)klass;
    khint_t k = kh_get(sigoffset, my_sigoffset, itype);
    if(k==kh_end(my_sigoffset))
        return;
    sigoffset_array_t *p = &kh_value(my_sigoffset, k);
    printf_log(LOG_DEBUG, "my_unwrap_signal_offset(%p) type=0x%zx with %d signals with offset\n", klass, itype, p->sz);
    for(int i=0; i<p->sz; ++i) {
        void** f = (void**)((uintptr_t)klass + p->a[i].offset);
        void* new_f = GetNativeFnc((uintptr_t)*f);
        if(!new_f) {
            // Not a native function: autobridge it
            new_f = finders[p->a[i].n](f);
        }
        if(new_f != *f) {
            printf_log(LOG_DEBUG, "Unwrapping %p[%p: offset=0x%x, n=%d] -> %p (with alternate)\n", *f, f, p->a[i].offset, p->a[i].n, new_f);
            if(!hasAlternate(new_f))
                addAlternate(new_f, *f);
            *f = new_f;
        }
    }
}

// First the structure my_GTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GTypeInfo_t     my_gtypeinfo_##A = {0};   \
static my_GTypeInfo_t    ref_gtypeinfo_##A = {0};   \
static int              used_gtypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
// base_init ...
#define GO(A)   \
static uintptr_t my_base_init_fct_##A = 0;                          \
static int my_base_init_##A(void* a)                                \
{                                                                   \
    return RunFunctionFmt(my_base_init_fct_##A, "p", a);     \
}
SUPER()
#undef GO
static void* find_base_init_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_init_fct_##A == (uintptr_t)fct) return my_base_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_init_fct_##A == 0) {my_base_init_fct_##A = (uintptr_t)fct; return my_base_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_init callback\n");
    return NULL;
}
// base_finalize ...
#define GO(A)   \
static uintptr_t my_base_finalize_fct_##A = 0;                      \
static int my_base_finalize_##A(void* a)                            \
{                                                                   \
    return RunFunctionFmt(my_base_finalize_fct_##A, "p", a); \
}
SUPER()
#undef GO
static void* find_base_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_base_finalize_fct_##A == (uintptr_t)fct) return my_base_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_base_finalize_fct_##A == 0) {my_base_finalize_fct_##A = (uintptr_t)fct; return my_base_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo base_finalize callback\n");
    return NULL;
}
// class_init ...
#define GO(A)   \
static uintptr_t my_class_init_fct_##A = 0;                                 \
static size_t parent_class_init_##A = 0;                                    \
static int my_class_init_##A(void* a, void* b)                              \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Class init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_class_init_##A, g_type_name(parent_class_init_##A));\
    int ret = RunFunctionFmt(my_class_init_fct_##A, "pp", a, b);            \
    size_t type = parent_class_init_##A;                                    \
    while(checkRegisteredClass(type))                                       \
        type = g_type_parent(type);                                         \
    unwrapGTKClass(a, type);                                                \
    my_unwrap_signal_offset(a);                                             \
    if(!strcmp(g_type_name(type), "AtkUtil")) {                             \
        my_AtkUtilClass_t* p = (my_AtkUtilClass_t*)g_type_class_peek(type); \
        unwrapGTKClass(p, type);                                            \
    }                                                                       \
    return ret;                                                             \
}
SUPER()
#undef GO
static void* find_class_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_init_fct_##A == (uintptr_t)fct && parent_class_init_##A==parent) return my_class_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_init_fct_##A == 0) {my_class_init_fct_##A = (uintptr_t)fct; parent_class_init_##A=parent; return my_class_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_init callback\n");
    return NULL;
}
// class_finalize ...
#define GO(A)   \
static uintptr_t my_class_finalize_fct_##A = 0;                                 \
static int my_class_finalize_##A(void* a, void* b)                              \
{                                                                               \
    return RunFunctionFmt(my_class_finalize_fct_##A, "pp", a, b);               \
}
SUPER()
#undef GO
static void* find_class_finalize_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_class_finalize_fct_##A == (uintptr_t)fct) return my_class_finalize_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_class_finalize_fct_##A == 0) {my_class_finalize_fct_##A = (uintptr_t)fct; return my_class_finalize_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo class_finalize callback\n");
    return NULL;
}
// instance_init ...
#define GO(A)   \
static uintptr_t my_instance_init_fct_##A = 0;                              \
static size_t parent_instance_init_##A = 0;                                 \
static int my_instance_init_##A(void* a, void* b)                           \
{                                                                           \
    printf_log(LOG_DEBUG, "Custom Instance init %d for class %p (parent=%p:%s)\n", A, a, (void*)parent_instance_init_##A, g_type_name(parent_instance_init_##A));\
    int ret = RunFunctionFmt(my_instance_init_fct_##A, "pp", a, b);         \
    size_t type = parent_instance_init_##A;                                 \
    while(checkRegisteredClass(type))                                       \
        type = g_type_parent(type);                                         \
    unwrapGTKInstance(a, type);                                             \
    bridgeGTKInstance(a, type);                                             \
    return ret;                                                             \
}
SUPER()
#undef GO
static void* find_instance_init_Fct(void* fct, size_t parent)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_instance_init_fct_##A == (uintptr_t)fct && parent_instance_init_##A==parent) return my_instance_init_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_instance_init_fct_##A == 0) {my_instance_init_fct_##A = (uintptr_t)fct; parent_instance_init_##A=parent; return my_instance_init_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo instance_init callback\n");
    return NULL;
}
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GTypeInfo_t* findFreeGTypeInfo(my_GTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtypeinfo_##A && memcmp(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t))==0) return &my_gtypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtypeinfo_##A == 0) {                                                 \
        used_gtypeinfo_##A = 1;                                                                 \
        memcpy(&ref_gtypeinfo_##A, fcts, sizeof(my_GTypeInfo_t));                               \
        my_gtypeinfo_##A.class_size = fcts->class_size;                                         \
        my_gtypeinfo_##A.base_init = find_base_init_Fct(fcts->base_init);                       \
        my_gtypeinfo_##A.base_finalize = find_base_finalize_Fct(fcts->base_finalize);           \
        my_gtypeinfo_##A.class_init = find_class_init_Fct(fcts->class_init, parent);            \
        my_gtypeinfo_##A.class_finalize = find_class_finalize_Fct(fcts->class_finalize);        \
        my_gtypeinfo_##A.class_data = fcts->class_data;                                         \
        my_gtypeinfo_##A.instance_size = fcts->instance_size;                                   \
        my_gtypeinfo_##A.n_preallocs = fcts->n_preallocs;                                       \
        my_gtypeinfo_##A.instance_init = find_instance_init_Fct(fcts->instance_init, parent);   \
        my_gtypeinfo_##A.value_table = findFreeGTypeValueTable(fcts->value_table);              \
        return &my_gtypeinfo_##A;                                                               \
    }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GTypeInfo callback\n");
    return NULL;
}

// ---- GtkTypeInfo ----

// First the structure my_GtkTypeInfo_t statics, with paired x64 source pointer
#define GO(A) \
static my_GtkTypeInfo_t     my_gtktypeinfo_##A = {0};   \
static my_GtkTypeInfo_t    ref_gtktypeinfo_##A = {0};  \
static int                used_gtktypeinfo_##A = 0;
SUPER()
#undef GO
// Then the static functions callback that may be used with the structure
#define GO(A)   \
static int fct_gtk_parent_##A = 0 ;                                                 \
static uintptr_t fct_gtk_class_init_##A = 0;                                        \
static int my_gtk_class_init_##A(void* g_class) {                                   \
    printf_log(LOG_DEBUG, "Calling fct_gtk_class_init_" #A " wrapper\n");           \
    int ret = (int)RunFunctionFmt(fct_gtk_class_init_##A, "p", g_class);            \
    unwrapGTKClass(g_class, fct_gtk_parent_##A);                                    \
    return ret;                                                                     \
}                                                                                   \
static uintptr_t fct_gtk_object_init_##A = 0;                                       \
static int my_gtk_object_init_##A(void* object, void* data) {                       \
    printf_log(LOG_DEBUG, "Calling fct_gtk_object_init_" #A " wrapper\n");          \
    return (int)RunFunctionFmt(fct_gtk_object_init_##A, "pp", object, data);        \
}                                                                                   \
static uintptr_t fct_gtk_base_class_init_##A = 0;                                   \
static int my_gtk_base_class_init_##A(void* instance, void* data) {                 \
    printf_log(LOG_DEBUG, "Calling fct_gtk_base_class_init_" #A " wrapper\n");      \
    return (int)RunFunctionFmt(fct_gtk_base_class_init_##A, "pp", instance, data);  \
}

SUPER()
#undef GO
// And now the get slot / assign... Taking into account that the desired callback may already be a wrapped one (so unwrapping it)
my_GtkTypeInfo_t* findFreeGtkTypeInfo(my_GtkTypeInfo_t* fcts, size_t parent)
{
    if(!fcts) return NULL;
    #define GO(A) if(used_gtktypeinfo_##A && memcmp(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t))==0) return &my_gtktypeinfo_##A;
    SUPER()
    #undef GO
    #define GO(A) if(used_gtktypeinfo_##A == 0) {          \
        memcpy(&ref_gtktypeinfo_##A, fcts, sizeof(my_GtkTypeInfo_t));        \
        fct_gtk_parent_##A = parent;                        \
        my_gtktypeinfo_##A.type_name = fcts->type_name; \
        my_gtktypeinfo_##A.object_size = fcts->object_size; \
        my_gtktypeinfo_##A.class_size = fcts->class_size; \
        my_gtktypeinfo_##A.class_init_func = (fcts->class_init_func)?((GetNativeFnc((uintptr_t)fcts->class_init_func))?GetNativeFnc((uintptr_t)fcts->class_init_func):(void*)my_gtk_class_init_##A):NULL;    \
        fct_gtk_class_init_##A = (uintptr_t)fcts->class_init_func;           \
        my_gtktypeinfo_##A.object_init_func = (fcts->object_init_func)?((GetNativeFnc((uintptr_t)fcts->object_init_func))?GetNativeFnc((uintptr_t)fcts->object_init_func):(void*)my_gtk_object_init_##A):NULL;    \
        fct_gtk_object_init_##A = (uintptr_t)fcts->object_init_func;         \
        my_gtktypeinfo_##A.reserved_1 = fcts->reserved_1;                 \
        my_gtktypeinfo_##A.reserved_2 = fcts->reserved_2;                 \
        my_gtktypeinfo_##A.base_class_init_func = (fcts->base_class_init_func)?((GetNativeFnc((uintptr_t)fcts->base_class_init_func))?GetNativeFnc((uintptr_t)fcts->base_class_init_func):(void*)my_gtk_base_class_init_##A):NULL;    \
        fct_gtk_base_class_init_##A = (uintptr_t)fcts->base_class_init_func;   \
        return &my_gtktypeinfo_##A;                       \
    }

    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for GtkTypeInfo callback\n");
    return NULL;
}

#undef SUPER

void InitGTKClass(bridge_t *bridge)
{
    my_bridge  = bridge;
    my_signalmap = kh_init(signalmap);
    my_sigoffset = kh_init(sigoffset);
}

void FiniGTKClass()
{
    if(my_signalmap) {
        /*khint_t k;
        kh_foreach_key(my_signalmap, k,
            my_signal_t* p = (my_signal_t*)(uintptr_t)k;
            box_free(p);
        );*/ // lets assume all signals data is freed by gtk already
        kh_destroy(signalmap, my_signalmap);
        my_signalmap = NULL;
    }
    if(my_sigoffset) {
        sigoffset_array_t* p;
        kh_foreach_value_ref(my_sigoffset, p,
            box_free(p->a);
        );
        kh_destroy(sigoffset, my_sigoffset);
        my_sigoffset = NULL;
    }
}

#define GTKCLASS(A)             \
void Set##A##ID(size_t id)      \
{                               \
    my_##A = id;                \
}
#define GTKIFACE(A)  GTKCLASS(A)
GTKCLASSES()
#undef GTKIFACE
#undef GTKCLASS

void AutoBridgeGtk(void*(*ref)(size_t), void(*unref)(void*))
{
    void* p;
    #define GTKIFACE(A)
    #define GTKCLASS(A)                \
    if(my_##A && my_##A!=(size_t)-1) { \
        p = ref(my_##A);               \
        bridgeGTKClass(p, my_##A);     \
        unref(p);                      \
    }
    GTKCLASSES()
    #undef GTKIFACE
    #undef GTKCLASS
}

void SetGTypeName(void* f)
{
    g_type_name = f;
}

void SetGClassPeek(void* f)
{
    g_type_class_peek = f;
}

void SetGTypeParent(void* f)
{
    g_type_parent = f;
}

