// SPDX-License-Identifier: MIT
#ifndef _BOX64_LIBC_DIRENT_H_
#define _BOX64_LIBC_DIRENT_H_

#ifndef HAVE_SCANDIRAT
int scandirat(int dirfd, const char *dirp, struct dirent ***namelist, 
              int (*filter)(const struct dirent *),int (*compar)(const struct dirent**, const struct dirent **));
#endif

#endif