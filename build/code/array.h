#if !defined(ARRAY_H)
#define ARRAY_H

#define ArrayPush(array, value, type)                              \
    do                                                             \
    {                                                              \
        (array) = (type*)ArrayHold((array), 1, sizeof(*(array)));  \
        (array)[ArrayLength(array) - 1] = (value);                 \
    } while(0);

void* ArrayHold(void* Array, int Count, int ItemSize);
int ArrayLength(void* Array);
void ArrayFree(void* Array);

#endif
