#pragma once

#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/** Thread-unsafe string implementation, with support for raw NULL bytes
 * USAGE:
 * ======
 * string_t ------------ The string type.
 * string_new ---------- Creates a new string.
 * string_new_cap ------ Creates a new string with a given capacity. Takes the capacity.
 * string_new_cstr ----- Creates a new string from a given C string. Takes the string.
 * string_reserve ------ Ensures a string has at least a given capacity. Takes the string and the capacity.
 * string_trim --------- Ensures a string has a capacity equal to its length. Takes the string. May reduce the string capacity.
 * string_del ---------- Frees a string. Takes the string.
 * string_steal -------- Frees a string, keeping the content alive. Takes the string. The content (also returned) needs to be freed separately.
 * string_add_char ----- Add a character at the end. Takes the string and the new character. May increase the string capacity.
 * string_add_char_at -- Add a character at a given index. Takes the string, the new character and the index. May increase the string capacity.
 * string_add_string --- Add a string at the end in-place. Takes both strings. May increase the string capacity.
 * string_add_cstr ----- Add a C string at the end in-place. Takes both strings. May increase the string capacity.
 * string_pop ---------- Pops the last character. Takes the string. May reduce the string capacity.
 * string_clear -------- Clears the string. Takes the string. May reduce the string capacity.
 * string_dup ---------- Duplicate a string. Takes the string. Does not free the old string.
 * string_concat ------- Concatenate two strings. Takes both strings. Does not free any string.
 * string_len ---------- String length. Takes the string.
 * string_cap ---------- String capacity. Takes the string.
 * string_content ------ Pointer to the string content. Valid C string. Takes the string.
 * string_begin -------- Start of the string. Takes the string.
 * string_end ---------- End of the string. Points to unmanaged memory. Takes the string.
 * string_last --------- Last element of the string. Points to invalid memory if size is zero. Takes the string.
 * string_for ---------- Iterate over the characters of a string. This is a for loop. Takes the iterator name and the string.
 * 
 * EXAMPLE:
 * ========
 * Source main.c:
 * -------------------
// ...
int main() {
	string_t *str = string_new_cap(2);
	if (!str) {
		printf("Error: failed to allocate new string\n");
		return 2;
	}
	string_add_char(str, 'H'); // Cannot fail
	string_add_char(str, 'i'); // Cannot fail
	if (!string_add_char(str, '!')) {
		printf("Error: failed to add char to string\n");
		return 2;
	}
	printf("String length: %zu: \"%s\"\n", string_len(str), string_content(str)); // 3, "Hi!"
	string_del(str);
}
 */

typedef struct string_s {
	size_t ssize, scap;
	char *buf;
} string_t;

string_t *string_new(void);
string_t *string_new_cap(size_t cap);
string_t *string_new_cstr(const char *s);
int       string_reserve(string_t *s, size_t cap);
int       string_trim(string_t *s);
void      string_del(string_t *s);
char     *string_steal(string_t *s);
int       string_add_char(string_t *s, char elem);
int       string_add_char_at(string_t *s, char elem, size_t idx);
int       string_add_string(string_t *s1, string_t *s2);
int       string_add_cstr(string_t *s1, const char *s2);
void      string_pop(string_t *s);
void      string_clear(string_t *s);
string_t *string_dup(string_t const *s);
string_t *string_concat(string_t const *l, string_t const *r);
#define string_len(s) ((s)->ssize)
#define string_cap(s) ((s)->scap)
#define string_content(s) ((s)->buf)
#define string_begin(s) ((s)->buf)
#define string_end(s) ((s)->buf + (s)->ssize)
#define string_last(s) ((s)->buf[(s)->ssize - 1])
#define string_for(itname, s) \
	for (char *itname = string_begin((s)); itname != string_end((s)); ++itname)

#endif // STRING_H
