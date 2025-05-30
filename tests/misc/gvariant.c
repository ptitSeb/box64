// gcc `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -O1 gvariant.c -o gvariant
#include <glib.h>

static void print(GVariant* variant)
{
    if (variant != NULL) {
        gchar* str = g_variant_print(variant, TRUE);
        g_print("%s\n", str);
        g_free(str);
        g_variant_unref(variant);
    } else {
        g_print("Failed to create GVariant\n");
    }
}

static void func(const gchar* format, ...)
{
    GVariant* variant = NULL;
    va_list ap;
    va_start(ap, format);
    variant = g_variant_new_va(format, NULL, &ap);
    va_end(ap);

    print(variant);
}

int main(int argc, char* argv[])
{
    func("(bynqiuxthiiiiiiiiiiiiii)", FALSE, 'A', 3, 4, 5, 6, G_GINT64_CONSTANT(7), G_GUINT64_CONSTANT(8), 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23);
    func("(bynqiuxthdiiiiiiiiidiii)", TRUE, 'A', 3, 4, 5, 6, G_GINT64_CONSTANT(7), G_GUINT64_CONSTANT(8), 9, 10.1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20.2, 21, 22, 23);
    func("(bdididxdidididididididi)", TRUE, 2.2, 3, 4.4, 5, 6.6, G_GINT64_CONSTANT(7), 8.8, 9, 10.1, 11, 12.12, 13, 14.14, 15, 16.16, 17, 18.18, 19, 20.2, 21, 22.22, 23);
    func("(bdididxdididididiiiiiii)", TRUE, 2.2, 3, 4.4, 5, 6.6, G_GINT64_CONSTANT(7), 8.8, 9, 10.1, 11, 12.12, 13, 14.14, 15, 16.16, 17, 18, 19, 20, 21, 22, 23);
    func("(biidiixdiiidiiidiiidiii)", TRUE, 2, 3, 4.4, 5, 6, G_GINT64_CONSTANT(7), 8.8, 9, 10, 11, 12.12, 13, 14, 15, 16.16, 17, 18, 19, 20.2, 21, 22, 23);
    func("(ddddddddddddddddddddddd)", 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12, 13.13, 14.14, 15.15, 16.16, 17.17, 18.18, 19.19, 20.2, 21.21, 22.22, 23.23);
    GVariant* variant = g_variant_new("(bynqiuxthiiiiiiiiiiiiii)", FALSE, 'A', 3, 4, 5, 6, G_GINT64_CONSTANT(7), G_GUINT64_CONSTANT(8), 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23);
    print(variant);
    variant = g_variant_new("(bynqiuxthdiiiiiiiiidiii)", TRUE, 'A', 3, 4, 5, 6, G_GINT64_CONSTANT(7), G_GUINT64_CONSTANT(8), 9, 10.1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20.2, 21, 22, 23);
    print(variant);
    variant = g_variant_new("(bdididxdidididididididi)", TRUE, 2.2, 3, 4.4, 5, 6.6, G_GINT64_CONSTANT(7), 8.8, 9, 10.1, 11, 12.12, 13, 14.14, 15, 16.16, 17, 18.18, 19, 20.2, 21, 22.22, 23);
    print(variant);
    variant = g_variant_new("(bdididxdididididiiiiiii)", TRUE, 2.2, 3, 4.4, 5, 6.6, G_GINT64_CONSTANT(7), 8.8, 9, 10.1, 11, 12.12, 13, 14.14, 15, 16.16, 17, 18, 19, 20, 21, 22, 23);
    print(variant);
    variant = g_variant_new("(biidiixdiiidiiidiiidiii)", TRUE, 2, 3, 4.4, 5, 6, G_GINT64_CONSTANT(7), 8.8, 9, 10, 11, 12.12, 13, 14, 15, 16.16, 17, 18, 19, 20.2, 21, 22, 23);
    print(variant);
    variant = g_variant_new("(ddddddddddddddddddddddd)", 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12, 13.13, 14.14, 15.15, 16.16, 17.17, 18.18, 19.19, 20.2, 21.21, 22.22, 23.23);
    print(variant);
    return 0;
}
