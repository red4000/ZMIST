#ifndef _VECTOR_H
#define _VECTOR_H

#include <stdlib.h>
//#define CHECK_BOUNDS

template <typename T, int step = 16>
class Vector {
public:
	T     *base;
	size_t usedCapacity, maxCapacity;

	inline Vector() {
		base         = NULL;
		usedCapacity = maxCapacity = 0;
	}

	inline Vector(Vector<T> *v) {
		if (0 != v->maxCapacity) {
			base         = (T*)malloc(v->maxCapacity * sizeof(T));
			usedCapacity = v->usedCapacity;
			maxCapacity  = v->maxCapacity;
		} else {
			base         = NULL;
			usedCapacity = maxCapacity = 0;
		}
	}

	inline Vector(size_t initialCapacity) {
		maxCapacity  = initialCapacity;
		base         = (T*)malloc(initialCapacity * sizeof(T));
		usedCapacity = 0;
	}

	inline ~Vector() {
		if (NULL != base) {
			//free(base);
		}
	}

	typedef T* iterator;

	inline void merge(Vector<T> *v) {
		for (size_t i = 0; i < v->size(); i++) {
			if (0 == find(v->base[i])) {
				push_back(v->base[i]);
			}
		}
	}

	iterator begin() {
		return (iterator)base;
	}

	iterator end() {
		return (Vector<T>::iterator)(((size_t)base) + (usedCapacity * sizeof(T)));
	}

	iterator at(size_t i) {
#ifdef CHECK_BOUNDS
		if (i < usedCapacity) {
#endif
		return (Vector<T>::iterator)(((size_t)base) + (i * sizeof(T)));
#ifdef CHECK_BOUNDS
	}

	return NULL;
#endif
	}

	iterator insert(iterator at, T *val) {
		if ((usedCapacity + 1) >= maxCapacity) {
			reserve(maxCapacity + step);
		}

		memmove((void*)at(i + 1), (void*)at(i), (1 + (usedCapacity - i)) * sizeof(T));
		(*at) = *val;
		usedCapacity++;
	}

	inline size_t index(iterator it) {
		return (((size_t)it) - ((size_t)base)) / sizeof(T);
	}

	inline size_t size() {
		return usedCapacity;
	}

	inline size_t capacity() {
		return maxCapacity;
	}

	inline bool empty() {
		return 0 == usedCapacity;
	}

	inline T *front() {
		return (T*)base;
	}

	inline T *back() {
		return (T*)(((size_t)base) + (usedCapacity * sizeof(T)));
	}

	inline void push_back(T *value) {
		if ((usedCapacity + 1) >= maxCapacity) {
			reserve(maxCapacity + step);
		}

		((T*)base)[++usedCapacity] = *value;
	}

	inline void push_back(T value) {
		if ((usedCapacity + 1) >= maxCapacity) {
			reserve(maxCapacity + step);
		}

		((T*)base)[usedCapacity++] = value;
	}

	inline T *push_new() {
		if ((usedCapacity + 1) >= maxCapacity) {
			reserve(maxCapacity + step);
		}

		return &((T*)base)[usedCapacity++];
	}

	inline void pop_back() {
		if (0 != usedCapacity) {
			usedCapacity--;
		}
	}

	inline void reserve(size_t cap) {
		maxCapacity = cap;
		base        = (T*)realloc(base, cap * sizeof(T));
	}

	inline void clear() {
		usedCapacity = 0;
	}

	inline void remove(size_t i) {
		if (usedCapacity != i) {
			memmove((void*)at(i), (void*)at(i + 1), (1 + (usedCapacity - i)) * sizeof(T));
		}

		usedCapacity--;
	}

	inline void erase(iterator it) {
		remove(index(it));
	}

	inline iterator find(T value) {
		for (iterator it = begin(); it != end(); it++) {
			if (value == *it) {
				return it;
			}
		}

		return NULL;
	}

	inline T operator[](size_t i) {
#ifdef CHECK_BOUNDS
		if (i < usedCapacity) {
#endif
		return ((T*)base)[i];
#ifdef CHECK_BOUNDS
	}

	return NULL;
#endif
	}

	inline Vector<T> *operator=(Vector<T> *v) {
		if (0 != v->usedCapacity) {
			base         = (T*)realloc(base, v->maxCapacity * sizeof(T));
			usedCapacity = v->usedCapacity;
			maxCapacity  = v->maxCapacity;
			memcpy(base, v->base, v->usedCapacity * sizeof(T));
		} else {
			base         = NULL;
			usedCapacity = maxCapacity = 0;
		}
	}
};
#endif
