#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void loginfo_print(const loginfo_t *info, int print_sz) {
	if (!info) {
		if (print_sz > 0) printf("%*s", print_sz, "");
		return;
	}
	if (!info->filename) {
		if (print_sz > 0) printf("%*s", print_sz, "");
		return;
	}
	print_sz -= printf("%s:", info->filename);
	if (!info->lineno) {
		print_sz -= printf(" ");
		if (print_sz > 0) printf("%*s", print_sz, "");
		return;
	}
	print_sz -= printf("%zu:", info->lineno);
	if (!info->colno) {
		print_sz -= printf(" ");
		if (print_sz > 0) printf("%*s", print_sz, "");
		return;
	}
	print_sz -= printf("%zu", info->colno);
	if (!info->lineno_end || ((info->lineno_end == info->lineno) && (info->colno_end == info->colno))) {
		print_sz -= printf(": ");
		if (print_sz > 0) printf("%*s", print_sz, "");
		return;
	}
	print_sz -= printf("-");
	if (info->lineno_end != info->lineno) {
		print_sz -= printf("%zu:", info->lineno_end);
	}
	print_sz -= printf("%zu: ", info->colno_end + 1);
	if (print_sz > 0) printf("%*s", print_sz, "");
}

void log_error(const loginfo_t *info, const char *format, ...) {
	printf("Error: ");
	loginfo_print(info, 0);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
void log_internal(const loginfo_t *info, const char *format, ...) {
	printf("Internal error: ");
	loginfo_print(info, 0);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
void log_memory(const char *format, ...) {
	printf("Fatal error: memory error: ");
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
void log_TODO(const loginfo_t *info, const char *format, ...) {
	printf("Error: ");
	loginfo_print(info, 0);
	printf("TODO: ");
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
void log_warning(const loginfo_t *info, const char *format, ...) {
	printf("Warning: ");
	loginfo_print(info, 0);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
