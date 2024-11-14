#include <glib.h>

static void func(const gchar* format, ...) {
  va_list ap;
  va_start(ap, format);
  g_variant_new_va(format, NULL, &ap);
  va_end(ap);
}

int main(int argc, char* argv[]) {
  func("(bynqiuxthiiiiiiiiiiiiii)", TRUE, 'A', 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23);
  func("(bynqiuxthdiiiiiiiiidiii)", TRUE, 'A', 3, 4, 5, 6, 7, 8, 9, 10.1, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20.2, 21, 22, 23);
  func("(bdididxdidididididididi)", TRUE, 2.2, 3, 4.4, 5, 6.6, 7, 8.8, 9, 10.1, 11, 12.12, 13, 14.14, 15, 16.16, 17, 18.18, 19, 20.2, 21, 22.22, 23);
  func("(bdididxdididididiiiiiii)", TRUE, 2.2, 3, 4.4, 5, 6.6, 7, 8.8, 9, 10.1, 11, 12.12, 13, 14.14, 15, 16.16, 17, 18, 19, 20, 21, 22, 23);
  func("(biidiixdiiidiiidiiidiii)", TRUE, 2, 3, 4.4, 5, 6, 7, 8.8, 9, 10, 11, 12.12, 13, 14, 15, 16.16, 17, 18, 19, 20.2, 21, 22, 23);
  func("(ddddddddddddddddddddddd)", 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12, 13.13, 14.14, 15.15, 16.16, 17.17, 18.18, 19.19, 20.2, 21.21, 22.22, 23.23);
  return 0;
}
