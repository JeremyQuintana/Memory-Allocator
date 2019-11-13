#include <unistd.h>
#include <list>
#include <iterator>
#include <iostream>
#include "Chunk.cpp"
#include "Locker.h"

using std::list;
using std::cout;
using std::endl;

void* alloc(size_t chunk_size);
void dealloc(void* chunk);

void setVersion(int versionPass);
int getVersion();
void printLists();
void printListSizes();
