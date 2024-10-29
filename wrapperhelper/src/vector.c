#include "vector.h"

VECTOR_IMPL(voidp, (void))
VECTOR_IMPL(char, (void))
VECTOR_IMPL(charp, (void))
VECTOR_IMPL(ccharp, (void))
static void stringp_del(string_t **s) { return string_del(*s); }
VECTOR_IMPL(string, stringp_del)

VECTOR(voidp) *vector_new_impl(void) {
	VECTOR(voidp) *ret = malloc(sizeof(*ret));
	if (!ret) return NULL;
	ret->vsize = ret->vcap = 0; ret->content_v = NULL;
	return ret;
}

VECTOR(voidp) *vector_new_cap_impl(size_t elem_size, size_t cap) {
	if (!cap) return vector_new_impl();
	VECTOR(voidp) *ret = malloc(sizeof(*ret));
	if (!ret) return NULL;
	cap = (cap < VECTOR_MIN_CAP) ? VECTOR_MIN_CAP : cap * 2;
	ret->content_v = malloc(cap * elem_size);
	if (!ret->content_v) {
		free(ret);
		return NULL;
	}
	ret->vcap = cap;
	ret->vsize = 0;
	return ret;
}

int vector_reserve_impl(VECTOR(voidp) *v, size_t elem_size, size_t cap) {
	size_t new_cap = (cap < VECTOR_MIN_CAP) ? VECTOR_MIN_CAP : cap;
	if (new_cap <= v->vcap) return 1;
	
	void *new_content_v = realloc(v->content_v, elem_size * new_cap);
	if (!new_content_v) return 0;
	v->content_v = new_content_v;
	v->vcap = new_cap;
	return 1;
}

int vector_trim_impl(VECTOR(voidp) *v, size_t elem_size) {
	if (v->vsize == v->vcap) return 1;
	if (v->vsize) {
		void *new_content_v = realloc(v->content_v, elem_size * v->vsize);
		if (!new_content_v) return 0;
		v->content_v = new_content_v;
		v->vcap = v->vsize;
		return 1;
	} else {
		free(v->content_v);
		v->content_v = NULL;
		v->vcap = 0;
		return 1;
	}
}

void vector_common_pop_impl(VECTOR(voidp) *v, size_t elem_size) {
	if (--v->vsize < v->vcap / 4) {
		size_t new_cap = (v->vcap / 2 < VECTOR_MIN_CAP) ? VECTOR_MIN_CAP : v->vcap / 2;
		if (new_cap == v->vcap) return;
		void *new_content_v = realloc(v->content_v, elem_size * new_cap);
		if (!new_content_v) return; // We don't really care if the realloc fails, we just need to not update anything
		v->content_v = new_content_v;
		v->vcap = new_cap;
	}
}

void vector_common_popn_impl(VECTOR(voidp) *v, size_t n, size_t elem_size) {
	if (n > v->vsize) n = v->vsize;
	v->vsize -= n;
	if (v->vsize < v->vcap / 4) {
		size_t new_cap = v->vcap / 2;
		while (v->vsize < new_cap / 4) new_cap /= 2;
		new_cap = (new_cap < VECTOR_MIN_CAP) ? VECTOR_MIN_CAP : new_cap;
		if (new_cap == v->vcap) return;
		void *new_content_v = realloc(v->content_v, elem_size * new_cap);
		if (!new_content_v) return; // We don't really care if the realloc fails, we just need to not update anything
		v->content_v = new_content_v;
		v->vcap = new_cap;
	}
}

void vector_common_clear_impl(VECTOR(voidp) *v) {
	if (!v->content_v) return;
	free(v->content_v);
	v->vsize = v->vcap = 0; v->content_v = NULL;
}

int vector_push_vec_impl(VECTOR(voidp) *v1, VECTOR(voidp) *v2, size_t start, size_t len, size_t elem_size) {
	if (start >= v2->vsize) return 1;
	if (start + len > v2->vsize) len = v2->vsize - start;
	if (!len) return 1;
	if (v1->vsize + len > v1->vcap) {
		size_t new_cap = (v1->vcap < VECTOR_MIN_CAP) ? VECTOR_MIN_CAP : v1->vcap * 2;
		while (v1->vsize + len > new_cap) {
			new_cap = new_cap * 2;
		}
		void *new_content_v = realloc(v1->content_v, elem_size * new_cap);
		if (!new_content_v) return 0;
		v1->content_v = new_content_v;
		v1->vcap = new_cap;
	}
	memcpy((char*)v1->content_v + elem_size * v1->vsize, (char*)v2->content_v + elem_size * start, elem_size * len);
	v1->vsize += len;
	return 1;
}
