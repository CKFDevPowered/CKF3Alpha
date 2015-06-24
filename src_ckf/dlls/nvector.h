#ifndef NVECTOR_H
#define NVECTOR_H

typedef struct
{
     void *items;
     size_t capacity;
     size_t size;
     size_t item_size;
}
vector_t;

typedef vector_t NagiVector;

vector_t *CreateVector(size_t item_size);
void DestroyVector(vector_t *v);
void *VectorGet(vector_t *v, size_t i);
size_t VectorSize(vector_t *v);
void VectorReserve(vector_t *v, size_t size);
void VectorClear(vector_t *v);
void VectorSet(vector_t *v, size_t i, void *item);
void VectorInsert(vector_t *v, size_t i, void *item);
void VectorEarse(vector_t *v, size_t start, size_t end);
void VectorPush(vector_t *v, void *item);
void VectorPop(vector_t *v);
void VectorPushData(vector_t *v, void *data, size_t count);
void VectorInsertData(vector_t *v, size_t i, void *data, size_t count);
void VectorSort(vector_t *self, int (*cmp)(const void *, const void *));

#endif