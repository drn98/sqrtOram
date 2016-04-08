#pragma once
#include<stddef.h>

// Sorts the part of data between indices [st,en)
// Uses cmpswap(data,i,j) to compare and swap data elements i and j
void batcherSort(void* data,size_t st,size_t en,
    void (*cmpswap)(void*,size_t,size_t));
