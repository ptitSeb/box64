// gcc gclosure.c -o gclosure `pkg-config --cflags --libs gobject-2.0`
#include <glib-object.h>

typedef void (*MyCallback)(gint arg1, const gchar* arg2, gpointer user_data);

void my_marshal(GClosure* closure,
    GValue* return_value,
    guint n_param_values,
    const GValue* param_values,
    gpointer invocation_hint,
    gpointer marshal_data)
{
    g_return_if_fail(n_param_values == 3);

    MyCallback callback;
    callback = (MyCallback)g_value_get_pointer(&param_values[0]);

    if (!callback)
        return;

    gint arg1 = g_value_get_int(&param_values[1]);
    const gchar* arg2 = g_value_get_string(&param_values[2]);

    callback(arg1, arg2, closure->data);
}

void example_callback(gint num, gchar* str, gpointer user_data)
{
    g_print("Callback called with: %d, '%s' (user_data: %p)\n",
        num, str, user_data);
}

int main()
{
    GClosure* closure = g_closure_new_simple(sizeof(GClosure), NULL);
    g_print("Initial refcount: %d\n", closure->ref_count);

    g_closure_set_marshal(closure, my_marshal);

    gpointer user_data = (gpointer)0x3a5000;
    closure->data = user_data;

    GValue params[3] = { G_VALUE_INIT, G_VALUE_INIT, G_VALUE_INIT };

    g_value_init(&params[0], G_TYPE_POINTER);
    g_value_set_pointer(&params[0], example_callback);

    g_value_init(&params[1], G_TYPE_INT);
    g_value_set_int(&params[1], 42);

    g_value_init(&params[2], G_TYPE_STRING);
    g_value_set_string(&params[2], "Hello GClosure");

    GValue return_value = G_VALUE_INIT;
    g_closure_invoke(closure, &return_value, 3, params, NULL);

    g_closure_ref(closure);
    g_print("After ref, refcount: %d\n", closure->ref_count);

    g_closure_sink(closure);
    g_print("After sinking, refcount: %d\n", closure->ref_count);

    g_closure_unref(closure);
    g_value_unset(&params[0]);
    g_value_unset(&params[1]);
    g_value_unset(&params[2]);

    return 0;
}
