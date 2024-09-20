#include "cstring.h"

#include <string.h>

#define STRING_MIN_CAP 8

string_t *string_new(void) {
	string_t *ret = malloc(sizeof(*ret));
	if (!ret) return NULL;
	char *buf = malloc(sizeof(char));
	if (!buf) {
		free(ret);
		return NULL;
	}
	buf[0] = '\0';
	ret->ssize = ret->scap = 0; ret->buf = buf;
	return ret;
}

string_t *string_new_cap(size_t cap) {
	string_t *ret = malloc(sizeof(*ret));
	if (!ret) return NULL;
	cap = (cap < STRING_MIN_CAP) ? STRING_MIN_CAP : cap;
	ret->buf = malloc((cap + 1) * sizeof(char));
	if (!ret->buf) {
		free(ret);
		return NULL;
	}
	ret->buf[0] = '\0';
	ret->scap = cap;
	ret->ssize = 0;
	return ret;
}

string_t *string_new_cstr(const char *s) {
	string_t *ret = malloc(sizeof(*ret));
	if (!ret) return NULL;
	size_t len = strlen(s);
	size_t cap = (len < STRING_MIN_CAP) ? STRING_MIN_CAP : len;
	ret->buf = malloc((cap + 1) * sizeof(char));
	if (!ret->buf) {
		free(ret);
		return NULL;
	}
	strcpy(ret->buf, s);
	ret->scap = cap;
	ret->ssize = len;
	return ret;
}

int string_reserve(string_t *s, size_t cap) {
	size_t new_cap = (cap < STRING_MIN_CAP) ? STRING_MIN_CAP : cap;
	if (new_cap <= s->scap) return 1;
	
	void *new_buf = realloc(s->buf, sizeof(char) * (new_cap + 1));
	if (!new_buf) return 0;
	s->buf = new_buf;
	s->scap = new_cap;
	return 1;
}
int string_reserve_grow(string_t *s, size_t cap) {
	cap = (cap < STRING_MIN_CAP) ? STRING_MIN_CAP : cap;
	if (cap <= s->scap) return 1;
	size_t new_cap = (s->scap < STRING_MIN_CAP) ? STRING_MIN_CAP : s->scap * 2;
	while (new_cap < cap) new_cap *= 2;
	
	void *new_buf = realloc(s->buf, sizeof(char) * (new_cap + 1));
	if (!new_buf) return 0;
	s->buf = new_buf;
	s->scap = new_cap;
	return 1;
}

int string_trim(string_t *s) {
	if (s->ssize == s->scap) return 1;
	void *new_buf = realloc(s->buf, sizeof(char) * (s->ssize + 1));
	if (!new_buf) return 0;
	s->buf = new_buf;
	s->scap = s->ssize;
	return 1;
}

void string_del(string_t *s) {
	if (s->buf) free(s->buf);
	free(s);
}

char *string_steal(string_t *s) {
	char *ret = s->buf;
	free(s);
	return ret;
}

int string_add_char(string_t *s, char elem) {
	if (!string_reserve_grow(s, s->ssize + 1)) return 0;
	s->buf[s->ssize++] = elem;
	s->buf[s->ssize] = '\0';
	return 1;
}

int string_add_char_at(string_t *s, char elem, size_t idx) {
	if (idx == s->ssize) return string_add_char(s, elem);
	if (idx > s->ssize) return 0;
	if (!string_reserve_grow(s, s->ssize + 1)) return 0;
	memmove(s->buf + idx + 1, s->buf + idx, s->ssize + 1 - idx);
	++s->ssize;
	s->buf[idx] = elem;
	return 1;
}

int string_add_string(string_t *s1, string_t *s2) {
	if (!string_reserve_grow(s1, s1->ssize + s2->ssize)) return 0;
	memcpy(s1->buf + s1->ssize, s2->buf, s2->ssize);
	s1->ssize += s2->ssize;
	s1->buf[s1->ssize] = '\0';
	return 1;
}

int string_add_cstr(string_t *s1, const char *s2) {
	size_t len = strlen(s2);
	if (!string_reserve_grow(s1, s1->ssize + len)) return 0;
	strcpy(s1->buf + s1->ssize, s2);
	s1->ssize += len;
	return 1;
}

void string_pop(string_t *s) {
	if (!s->ssize) return;
	s->buf[--s->ssize] = '\0';
	if (s->ssize < s->scap / 4) {
		size_t new_cap = (s->scap / 2 < STRING_MIN_CAP) ? STRING_MIN_CAP : s->scap / 2;
		if (new_cap == s->scap) return;
		void *new_buf = realloc(s->buf, sizeof(char) * (new_cap + 1));
		if (!new_buf) return; // We don't really care if the realloc fails, we just need to not update anything
		s->buf = new_buf;
		s->scap = new_cap;
	}
}

void string_clear(string_t *s) {
	if (!s->ssize) return;
	if (!s->scap) return;
	s->buf[s->ssize = 0] = '\0';
	void *new_buf = realloc(s->buf, sizeof(char));
	if (!new_buf) return; // We don't really care if the realloc fails, we just need to not update anything
	s->buf = new_buf;
	s->scap = 0;
}

string_t *string_dup(string_t const *s) {
	string_t *ret = string_new_cap(s->ssize);
	if (!ret) return NULL;
	memcpy(ret->buf, s->buf, s->ssize + 1);
	ret->ssize = s->ssize;
	return ret;
}

string_t *string_concat(string_t const *l, string_t const *r) {
	string_t *ret = string_new_cap(l->ssize + r->ssize);
	if (!ret) return NULL;
	memcpy(ret->buf, l->buf, l->ssize);
	memcpy(ret->buf + l->ssize, r->buf, r->ssize);
	ret->buf[ret->ssize] = '\0';
	return ret;
}
