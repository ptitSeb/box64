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

int string_reserve(string_t *s, size_t cap) {
	size_t new_cap = (cap < STRING_MIN_CAP) ? STRING_MIN_CAP : cap;
	if (new_cap <= s->scap) return 1;
	
	void *new_buf = realloc(s->buf, sizeof(char) * (new_cap + 1));
	if (!new_buf) return 0;
	s->buf = new_buf;
	s->scap = new_cap;
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
	if (s->ssize >= s->scap) {
		size_t new_cap = (s->scap < STRING_MIN_CAP) ? STRING_MIN_CAP : s->scap * 2;
		char *new_buf = realloc(s->buf, sizeof(char) * (new_cap + 1));
		if (!new_buf) return 0;
		s->buf = new_buf;
		s->scap = new_cap;
	}
	s->buf[s->ssize++] = elem;
	s->buf[s->ssize] = '\0';
	return 1;
}

int string_add_string(string_t *s1, string_t *s2) {
	if (s1->ssize + s2->ssize > s1->scap) {
		size_t new_cap = (s1->scap < STRING_MIN_CAP) ? STRING_MIN_CAP : s1->scap * 2;
		while (s1->ssize + s2->ssize > new_cap) {
			new_cap = new_cap * 2;
		}
		char *new_buf = realloc(s1->buf, sizeof(char) * (new_cap + 1));
		if (!new_buf) return 0;
		s1->buf = new_buf;
		s1->scap = new_cap;
	}
	memcpy(s1->buf + s1->ssize, s2->buf, s2->ssize);
	s1->ssize += s2->ssize;
	s1->buf[s1->ssize] = '\0';
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
