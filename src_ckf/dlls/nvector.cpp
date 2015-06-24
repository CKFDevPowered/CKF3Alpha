#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvector.h"

vector_t *CreateVector(size_t item_size)
{
	vector_t *v = (vector_t *)malloc(sizeof(vector_t));

	v->item_size = item_size;
	v->size = 0;
	v->capacity = 1;
	v->items = malloc(v->item_size * v->capacity);
	return v;
}

void DestroyVector(vector_t *v)
{
	free(v->items);
	free(v);
}

void *VectorGet(vector_t *v, size_t i)
{
	return (char *)(v->items) + i * v->item_size;
}
/*
void *VectorGet(vector_t *v, size_t i)
{
	return *(void **)((char *)(v->items) + i * v->item_size);
}
*/
size_t VectorSize(vector_t *v)
{
	return v->size;
}

void VectorReserve(vector_t *v, size_t size)
{
	if (v->capacity < size)
	{
		v->items = realloc(v->items, size * v->item_size);
		v->capacity = size;
	}
}

void VectorClear(vector_t *v)
{
	//v->size = 0;
	free(v->items);

	v->capacity = 1;
	v->items = malloc(v->item_size * v->capacity);
	v->size = 0;
}

void VectorSet(vector_t *v, size_t i, void *item)
{
	memcpy((char *)(v->items) + i * v->item_size, item, v->item_size);
}

void VectorInsert(vector_t *v, size_t i, void *item)
{
	if (v->capacity <= v->size)
		VectorReserve(v, 2 * v->capacity);

	if (i < v->size)
		memmove((char *)(v->items) + (i + 1) * v->item_size, (char *)(v->items) + (i + 0) * v->item_size, (v->size - i) * v->item_size);

	v->size++;
	VectorSet(v, i, item);
}

void VectorEarse(vector_t *v, size_t start, size_t end)
{
	memmove((char *)(v->items) + start * v->item_size, (char *)(v->items) + end * v->item_size, (v->size - end) * v->item_size);
	v->size -= (end - start);
}

void VectorPush(vector_t *v, void *item)
{
	VectorInsert(v, v->size, item);
}

void VectorPop(vector_t *v)
{
	v->size--;
}

void VectorPushData(vector_t *v, void *data, size_t count)
{
	if (v->capacity < (v->size + count))
		VectorReserve(v, v->size + count);

	memmove((char *)(v->items) + v->size * v->item_size, data, count * v->item_size);
	v->size += count;
}

void VectorInsertData(vector_t *v, size_t i, void *data, size_t count)
{
	if (v->capacity < (v->size + count))
		VectorReserve(v, v->size + count);

	memmove((char *)(v->items) + (i + count) * v->item_size, (char *)(v->items) + (i) * v->item_size, count * v->item_size);
	memmove((char *)(v->items) + i * v->item_size, data, count * v->item_size);
	v->size += count;
}

void VectorSort(vector_t *self, int (*cmp)(const void *, const void *))
{
    qsort(self->items, self->size, self->item_size, cmp);
}