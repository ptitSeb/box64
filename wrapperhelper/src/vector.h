#pragma once

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cstring.h"

// Note: do not use with empty types; simply use ints in this case.

/** Thread-unsafe vector implementation
 * USAGE:
 * ======
 * VECTOR_DECLARE ----------- Declare a new vector type. Takes the name and the element type.
 * VECTOR_IMPL -------------- Implements required functions for a vector type. Takes the name and the destructor of a pointer to the element.
 * VECTOR_DECLARE_STATIC ---- Declare a new vector type with static implementation. Takes the name and the element type.
 * VECTOR_IMPL_STATIC ------- Implements required functions for a vector type. Takes the name and the destructor of a pointer to the element.
 *                                Functions are declared static.
 * VECTOR_DEL --------------- Macro that takes the vector name and gives a function taking a vector and deletes it.
 * VECTOR ------------------- The vector type. Takes the name.
 * VECTOR_ELEM -------------- The element type. Takes the name.
 * vector_new --------------- Creates a new vector. Takes the name.
 * vector_new_cap ----------- Creates a new vector with a given capacity. Takes the name and the capacity.
 * vector_reserve ----------- Ensures a vector has at least a given capacity. Takes the name, the vector and the capacity. May not reduce the vector capacity.
 * vector_trim -------------- Ensures a vector has a capacity equal to its size. Takes the name and the vector. May reduce the vector capacity.
 * vector_del --------------- Frees a vector. Takes the name and the vector. Destructs the content of the vector.
 * vector_steal ------------- Frees a vector and returns the content. Takes the name and the vector. May reduce the vector capacity.
 * vector_del_freed --------- Frees a vector without freeing the content. Takes the name and the vector. Does not interact with the content of the vector.
 * vector_del_free_from ----- Frees a vector without freeing all of the content. Takes the name, the vector and the first element to free.
 *                                Destroys part of the content of the vector.
 * vector_push -------------- Push a new element. Takes the name, the vector and the new element. Does not fail if enough capacity remains.
 * vector_push_vec ---------- Push a vector of new elements. Takes the name, the vector and the new element vector. Does not fail if enough capacity remains.
 * vector_push_vec_slice ---- Push a slice of a vector of elements. Takes the name, the vector the new element vector, start and size.
 *                                Does not fail if enough capacity remains.
 * vector_pop --------------- Pops the last element. Takes the name and the vector. May reduce the vector capacity.
 * vector_pop_nodel --------- Pops the last element without freeing it. Takes the name and the vector. May reduce the vector capacity.
 * vector_pop_slice --------- Pops the last N element. Takes the name, the vector and the number of elements to remove.
 * vector_pop_nodel_slice --- Pops the last N element without freeing them. Takes the name, the vector and the number of elements to remove.
 *                                May reduce the vector capacity.
 * vector_clear ------------- Remove every element. Takes the name and the vector.
 * vector_clear_nodel ------- Remove every element without freeing them. Takes the name and the vector.
 * vector_remove ------------ Removes an element. Takes the name, the vector and the element number. May reduce the vector capacity.
 * vector_size -------------- Vector size (number of elements). Takes the name and the vector.
 * vector_cap --------------- Vector capacity (number of elements). Takes the name and the vector.
 * vector_content ----------- Pointer to the vector content. Takes the name and the vector.
 * vector_begin ------------- Start of the vector content. Takes the name and the vector.
 * vector_end --------------- End of the vector content. Points to unmanaged memory. Takes the name and the vector.
 * vector_last -------------- Last element of the vector. Points to invalid memory if size is zero. Takes the name and the vector.
 * vector_for --------------- Iterate over the elements of a vector. This is a for loop. Takes the name, the iterator name and the vector.
 * 
 * VECTOR_DEL(name)(v) is equivalent to vector_del(name, v).
 * Predefined vectors: string (string_t*), char (char), charp (char*), voidp (void*)
 * 
 * EXAMPLE:
 * ========
 * Header myvec.h:
 * ---------------
// ...
VECTOR_DECLARE(myvec, my_elem_t*)
// ...

 * Source myvec.c:
 * ---------------
// ...
VECTOR_IMPL(myvec)
// ...

 * Source main.c:
 * -------------------
// ...
extern my_elem_t elems[2];
int main() {
	VECTOR(myvec) *vec = vector_new_cap(myvec, 2);
	vector_push(myvec, vec, &elems[0]);
	vector_push(myvec, vec, &elems[1]);
	vector_for (myvec, it, vec) {
		printf("We have an element: %s\n", it->elem_name);
	}
	vector_del(myvec, vec);
}
 */

#define VECTOR(name) vector_##name##_t
#define VECTOR_ELEM(name) vector_##name##_elem

#define VECTOR_MIN_CAP 8

#define VECTOR_DECLARE_(name, t, pre) \
	typedef struct vector_##name##_s {                                        \
		size_t vsize;                                                         \
		size_t vcap;                                                          \
		union {                                                               \
			t *content;                                                       \
			void *content_v;                                                  \
		};                                                                    \
	} VECTOR(name);                                                           \
	typedef t VECTOR_ELEM(name);                                              \
	pre VECTOR_ELEM(name) *vector_steal_##name(VECTOR(name) *v);              \
	pre void vector_pop_del_##name(VECTOR(name) *v);                          \
	pre void vector_popn_del_##name(VECTOR(name) *v, size_t idx);             \
	pre void vector_clear_del_##name(VECTOR(name) *v);                        \
	pre void vector_del_from_##name(VECTOR(name) *v, size_t idx);             \
	pre int vector_push_elem_##name(VECTOR(name) *v, VECTOR_ELEM(name) elem);
#define VECTOR_DECLARE(name, t) VECTOR_DECLARE_(name, t,)
#define VECTOR_DECLARE_STATIC(name, t) VECTOR_DECLARE_(name, t, static)

VECTOR_DECLARE(voidp, void*)

VECTOR(voidp) *vector_new_impl(void);
VECTOR(voidp) *vector_new_cap_impl(size_t elem_size, size_t cap);
int            vector_reserve_impl(VECTOR(voidp) *v, size_t elem_size, size_t cap);
int            vector_trim_impl(VECTOR(voidp) *v, size_t elem_size);
void           vector_common_pop_impl(VECTOR(voidp) *v, size_t elem_size);
void           vector_common_popn_impl(VECTOR(voidp) *v, size_t n, size_t elem_size);
void           vector_common_clear_impl(VECTOR(voidp) *v);
int            vector_push_vec_impl(VECTOR(voidp) *v1, VECTOR(voidp) *v2, size_t start, size_t len, size_t elem_size);

#define vector_new(name) (VECTOR(name)*)vector_new_impl()
#define vector_new_cap(name, cap) (VECTOR(name)*)vector_new_cap_impl(sizeof(VECTOR_ELEM(name)), (cap))
#define vector_del(name, v) vector_del_from_##name((v), 0)
#define vector_del_freed(name, v) vector_del_from_##name((v), vector_size(name, (v)))
#define vector_del_free_from(name, v, i) vector_del_from_##name((v), (i))
#define VECTOR_DEL(name) vector_del_##name
#define vector_steal(name, v) vector_steal_##name((v))
#define vector_reserve(name, v, cap) vector_reserve_impl((VECTOR(voidp)*)(v), sizeof(VECTOR_ELEM(name)), (cap))
#define vector_trim(name, v) vector_trim_impl((VECTOR(voidp)*)(v), sizeof(VECTOR_ELEM(name)))
#define vector_push(name, v, e) vector_push_elem_##name((v), (e))
#define vector_push_vec(name, v1, v2) vector_push_vec_impl((VECTOR(voidp)*)(v1), (VECTOR(voidp)*)(v2), 0, vector_size(name, (v2)), sizeof(VECTOR_ELEM(name)))
#define vector_push_vec_slice(name, v1, v2, s, l) vector_push_vec_impl((VECTOR(voidp)*)(v1), (VECTOR(voidp)*)(v2), (s), (l), sizeof(VECTOR_ELEM(name)))
#define vector_pop(name, v) vector_pop_del_##name((v))
#define vector_pop_slice(name, v, n) vector_popn_del_##name((VECTOR(voidp)*)(v), (n))
#define vector_pop_nodel(name, v) vector_common_pop_impl((VECTOR(voidp)*)(v), sizeof(VECTOR_ELEM(name)))
#define vector_pop_nodel_slice(name, v, n) vector_common_popn_impl((VECTOR(voidp)*)(v), (n), sizeof(VECTOR_ELEM(name)))
#define vector_clear(name, v) vector_clear_del_##name((v))
#define vector_clear_nodel(name, v) vector_common_clear_impl((VECTOR(voidp)*)(v))
#define vector_remove(name, v, i) vector_remove_##name((v), (i))

#define vector_size(name, v) ((v)->vsize)
#define vector_cap(name, v) ((v)->vcap)
#define vector_content(name, v) ((v)->content)
#define vector_begin(name, v) ((v)->content)
#define vector_end(name, v) ((v)->content + (v)->vsize)
#define vector_last(name, v) ((v)->content[(v)->vsize - 1])
#define vector_for(name, itname, v) \
	for (VECTOR_ELEM(name) *itname = vector_begin(name, (v)); itname < vector_end(name, (v)); ++itname)
#define vector_for_from(name, itname, v, i) \
	for (VECTOR_ELEM(name) *itname = vector_begin(name, (v)) + (i); itname < vector_end(name, (v)); ++itname)
#define vector_for_rev(name, itname, v) \
	for (VECTOR_ELEM(name) *itname = (v)->content ? vector_end(name, (v)) - 1 : NULL; (v)->content && (itname >= vector_begin(name, (v))); --itname)

#define VECTOR_IMPL_(name, dtor, pre) \
	pre VECTOR_ELEM(name) *vector_steal_##name(VECTOR(name) *v) {                                      \
		vector_trim(name, v);                                                                          \
		VECTOR_ELEM(name) *ret = v->content;                                                           \
		free(v);                                                                                       \
		return ret;                                                                                    \
	}                                                                                                  \
	                                                                                                   \
	pre void vector_pop_del_##name(VECTOR(name) *v) {                                                  \
		if (v->vsize) {                                                                                \
			dtor(&vector_last(name, v));                                                               \
			vector_common_pop_impl((VECTOR(voidp)*)v, sizeof(VECTOR_ELEM(name)));                      \
		}                                                                                              \
	}                                                                                                  \
	pre void vector_popn_del_##name(VECTOR(name) *v, size_t n) {                                       \
		if (v->vsize > n) n = v->vsize;                                                                \
		vector_for_from(name, it, v, v->vsize - n) { dtor(it); }                                       \
		vector_common_popn_impl((VECTOR(voidp)*)v, n, sizeof(VECTOR_ELEM(name)));                      \
	}                                                                                                  \
	                                                                                                   \
	pre void vector_remove_##name(VECTOR(name) *v, size_t i) {                                         \
		dtor(v->content + i);                                                                          \
	    memmove(v->content + i, v->content + i + 1, (v->vsize - i - 1) * sizeof(VECTOR_ELEM(name)));   \
	    vector_common_pop_impl((VECTOR(voidp)*)v, sizeof(VECTOR_ELEM(name)));                          \
	}                                                                                                  \
	                                                                                                   \
	pre void vector_clear_del_##name(VECTOR(name) *v) {                                                \
		if (!v->content) return;                                                                       \
		vector_for(name, it, v) { dtor(it); }                                                          \
		free(v->content);                                                                              \
		v->content = NULL;                                                                             \
		v->vcap = v->vsize = 0;                                                                        \
	}                                                                                                  \
	                                                                                                   \
	pre void vector_del_from_##name(VECTOR(name) *v, size_t idx) {                                     \
		vector_for_from(name, it, v, idx) { dtor(it); }                                                \
		if (v->content) free(v->content);                                                              \
		free(v);                                                                                       \
	}                                                                                                  \
	                                                                                                   \
	pre int vector_push_elem_##name(VECTOR(name) *v, VECTOR_ELEM(name) elem) {                         \
		if (v->vsize >= v->vcap) {                                                                     \
			size_t new_cap = (v->vcap < VECTOR_MIN_CAP) ? VECTOR_MIN_CAP : v->vcap * 2;                \
			VECTOR_ELEM(name) *new_content = realloc(v->content, sizeof(VECTOR_ELEM(name)) * new_cap); \
			if (!new_content) return 0;                                                                \
			v->content = new_content;                                                                  \
			v->vcap = new_cap;                                                                         \
		}                                                                                              \
		v->content[v->vsize++] = elem;                                                                 \
		return 1;                                                                                      \
	}
#define VECTOR_IMPL(name, dtor) VECTOR_IMPL_(name, dtor,)
#define VECTOR_IMPL_STATIC(name, dtor) VECTOR_IMPL_(name, dtor, static inline)

VECTOR_DECLARE(char, char)
VECTOR_DECLARE(charp, char*)
VECTOR_DECLARE(ccharp, const char*)
VECTOR_DECLARE(string, string_t*)

#endif // VECTOR_H
