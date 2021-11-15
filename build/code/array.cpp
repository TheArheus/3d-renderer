
#include <stdio.h>
#include <stdlib.h>

#define ArrayRawData(array) ((int*)(array) - 2)
#define ArrayCapacity(array) (ArrayRawData(array)[0])
#define ArrayOccupied(array) (ArrayRawData(array)[1])

void* ArrayHold(void* Array, int Count, int ItemSize)
{
    if(Array == NULL)
    {
        int RawSize = (sizeof(int)*2) + (ItemSize * Count);
        int* Base = (int*)malloc(RawSize);
        Base[0] = Count; //Capacity
        Base[1] = Count; //Occupied
        return Base + 2;
    }
    else if(ArrayOccupied(Array) + Count <= ArrayCapacity(Array))
    {
        ArrayOccupied(Array) += Count;
        return Array;
    }
    else
    {
        int NeededSize = ArrayOccupied(Array) + Count;
        int FloatCurr = ArrayCapacity(Array) * 2;
        int Capacity = NeededSize > FloatCurr ? NeededSize : FloatCurr;
        int Occupied = NeededSize;
        int RawSize = sizeof(int) * 2 + ItemSize * Capacity;
        int* Base = (int*)realloc(ArrayRawData(Array), RawSize);
        Base[0] = Capacity;
        Base[1] = Occupied;
        return Base + 2;
    }
}

int ArrayLength(void* Array)
{
    return (Array != NULL) ? ArrayOccupied(Array) : 0;
}

void ArrayFree(void* Array)
{
    if(Array != NULL)
    {
        free(ArrayRawData(Array));
    }
}

