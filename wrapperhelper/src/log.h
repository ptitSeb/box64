#ifndef __LOG_H__
#define __LOG_H__

#include <stddef.h>

typedef struct loginfo_s {
	const char *filename; // NULL = no log info
	size_t lineno; // 0 = no (start) line/column number
	size_t colno;
	size_t lineno_end; // 0 = no end line/column number
	size_t colno_end;
} loginfo_t;

#define ATTRIBUTE_FORMAT(i, j) __attribute__((format(printf, i, j)))

void loginfo_print(const loginfo_t *info, int print_sz); // print_sz = min characters output
void log_error(const loginfo_t *info, const char *format, ...) ATTRIBUTE_FORMAT(2, 3);
void log_internal(const loginfo_t *info, const char *format, ...) ATTRIBUTE_FORMAT(2, 3);
void log_memory(const char *format, ...) ATTRIBUTE_FORMAT(1, 2);
void log_TODO(const loginfo_t *info, const char *format, ...) ATTRIBUTE_FORMAT(2, 3);
void log_warning(const loginfo_t *info, const char *format, ...) ATTRIBUTE_FORMAT(2, 3);
#define log_error_nopos(...) log_error(&(loginfo_t){0}, __VA_ARGS__)
#define log_internal_nopos(...) log_internal(&(loginfo_t){0}, __VA_ARGS__)
#define log_TODO_nopos(...) log_TODO(&(loginfo_t){0}, __VA_ARGS__)
#define log_warning_nopos(...) log_warning(&(loginfo_t){0}, __VA_ARGS__)

#endif // __LOG_H__
