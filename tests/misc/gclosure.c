// gcc gclosure.c -o gclosure `pkg-config --cflags --libs gobject-2.0`
#include <glib-object.h>

static void my_callback(gpointer data, gpointer user_data)
{
    g_print("Callback executed! Data: %s\n", (char*)user_data);
}

int main()
{
    GClosure* closure = g_cclosure_new(G_CALLBACK(my_callback),
        "Hello Closure!",
        NULL);
    g_print("Initial refcount: %d\n", closure->ref_count);

    g_closure_ref(closure);
    g_print("After ref, refcount: %d\n", closure->ref_count);

    g_closure_sink(closure);
    g_print("After sinking, refcount: %d\n", closure->ref_count);

    g_closure_unref(closure);
    g_print("After unref, refcount: %d\n", closure->ref_count);

    return 0;
}
