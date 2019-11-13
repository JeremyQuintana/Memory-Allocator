#include "Allocator.h"

//global variables
static int version;
static list<Chunk> allocatedList;
static list<Chunk> freeList;
static Locker allocLocker("Alloc");
static Locker freeLocker("Free");

//function prototypes
list<Chunk>::iterator firstFit(size_t chunk_size);
list<Chunk>::iterator bestFit(size_t chunk_size);
list<Chunk>::iterator worstFit(size_t chunk_size);
list<Chunk>::iterator split(size_t chunk_size, list<Chunk>::iterator chunk);

void* alloc(size_t chunk_size){
  //finds memory location
  //selects which version to run
  list<Chunk>::iterator memChunk;

  /*to accomodate for multithreading all threads search for a chunk and once
  found they will wait until all threads finish reading then a write Lock
  is placed to signify the chunk is being used
  if the chunk is being used after placing a write lock then the thread must
  search again*/
  bool noUniqueChunk = true;
  while (noUniqueChunk){
    freeLocker.readLock();
    //searches for chunk
    if (version == 1){
      memChunk = firstFit(chunk_size);
    } else if (version == 2){
      memChunk = bestFit(chunk_size);
    } else if (version == 3){
      memChunk = worstFit(chunk_size);
    }
    freeLocker.readUnlock();

    noUniqueChunk = false;

    freeLocker.writeLock();
    //checks if chunk is already taken by another thread
    //if so then it searches again for another chunk again
    if (memChunk != freeList.end()){
      if (memChunk->beingUsed == false){
        memChunk->beingUsed = true;
        noUniqueChunk = false;
      } else {
        noUniqueChunk = true;
      }
    }
    freeLocker.writeUnlock();
  }

  void* retChunk = nullptr;
  //if memory location is returns out of list then it allocates more memory
  if (memChunk == freeList.end()){
    //creates a new chunk
    Chunk newChunk;
    //places pointer of new memory through increasing heap size into new chunk
    //also places the size of how much the heap was increased by
    /*sbrk requires locks on both alloc and free as lists call sbrk too
    creating segmentation faults if context switched*/
    allocLocker.writeLock();
    newChunk.memoryAddress = sbrk(chunk_size);
    newChunk.chunkSize = chunk_size;

    //insert function returns an interator to area the new chunk is inserted
    //gets the memory address of the "chunk"
    retChunk = (allocatedList.insert(allocatedList.end(), newChunk))
                ->memoryAddress;
    allocLocker.writeUnlock();

  } else {
    freeLocker.writeLock();
    if (memChunk->chunkSize > (int)chunk_size){
      memChunk = split(chunk_size, memChunk);
    }
    freeLocker.writeUnlock();

    allocLocker.writeLock();
    retChunk = (allocatedList.insert(allocatedList.end(), *memChunk))
                ->memoryAddress;
    allocLocker.writeUnlock();

    freeLocker.writeLock();
    freeList.erase(memChunk);
    freeLocker.writeUnlock();

  }

  return retChunk;
}

void dealloc(void* chunk){
  bool found = false;

  allocLocker.writeLock();
  //iterates through the list to find the chunk
  for (list<Chunk>::iterator currChunk = allocatedList.begin();
       currChunk != allocatedList.end() && found == false; currChunk++){
    //if chunk is found move the chunk from allocated to free
    // cout << &*currChunk << endl;
    if (currChunk->memoryAddress == chunk){
      found = true;

      freeLocker.writeLock();

      allocatedList.erase(currChunk);
      allocLocker.writeUnlock();

      freeList.push_back(*currChunk);
      freeLocker.writeUnlock();
    }
  }


  if (found == false){
    allocLocker.writeUnlock();
    cout << "ERROR - Chunk not Found" << endl;
    abort();
  }

}

list<Chunk>::iterator firstFit(size_t chunk_size){
  list<Chunk>::iterator memChunk = freeList.end();

  //loops until a viable fit is found or no more memory
  bool found = false;
  for (list<Chunk>::iterator currChunk = freeList.begin();
       currChunk != freeList.end() && found == false; currChunk++){
    //if chunk big enough then it splits to only take needed chunk and is unused
    if (currChunk->chunkSize >= (int)chunk_size && currChunk->beingUsed == false){
      memChunk = currChunk;
      found = true;
    }
  }
  return memChunk;
}

list<Chunk>::iterator bestFit(size_t chunk_size){
  list<Chunk>::iterator memChunk = freeList.end();

  //if there are chunks in the free list
  if (freeList.size() != 0){
    //iterates through all chunks in free list
    bool found = false;
    list<Chunk>::iterator bestChunk;
    int smallestDiff = 0;
    for (list<Chunk>::iterator currChunk = freeList.begin();
         currChunk != freeList.end(); ++currChunk){
      int currDiff = (currChunk->chunkSize - chunk_size);

      //if the size of the chunk is big enough and unused
      if (currDiff >= 0 && currChunk->beingUsed == false){
        //if a chunk has not been found or its a better fit chunk
        if ((found == false) || (smallestDiff > currDiff)){
          //make the best chunk the current chunk
          bestChunk = currChunk;
          smallestDiff = currDiff;
          found = true;
        }
      }
    }

    //if a chunk has been found then make the returning chunk the best chunk
    if (found == true){
      memChunk = bestChunk;
    }
  }

  return memChunk;
}

list<Chunk>::iterator worstFit(size_t chunk_size){
  list<Chunk>::iterator memChunk = freeList.end();

  //if there are chunks in the free list
  if (freeList.size() != 0){

    //iterates through all chunks in free list
    bool found = false;
    list<Chunk>::iterator worstChunk;
    int largestDiff = 0;
    for (list<Chunk>::iterator currChunk = freeList.begin();
         currChunk != freeList.end(); ++currChunk){
      int currDiff = (currChunk->chunkSize - chunk_size);

      //if the size of the chunk is big enough and unused
      if (currDiff >= 0 && currChunk->beingUsed == false){
        //if a chunk has not been found or its a worse fit chunk
        if ((found == false) || (largestDiff < currDiff)){
          //make the worst chunk the current chunk
          worstChunk = currChunk;
          largestDiff = currDiff;
          found = true;
        }
      }
    }

    //if a chunk has been found then make the returning chunk the worst chunk
    if (found == true){
      memChunk = worstChunk;
    }
  }

  return memChunk;
}

//returns iterator of a chunk of given size that split from the larger chunk
list<Chunk>::iterator split(size_t chunk_size, list<Chunk>::iterator chunk){
  //create temporary chunk ptr to increment a given amount according to size
  char* tempChunkPtr = (char*)chunk->memoryAddress;
  tempChunkPtr += (chunk->chunkSize - chunk_size);
  //get the second pointer calculated from the temporary chunk ptr
  void* secondChunkPtr = (void*)tempChunkPtr;
  Chunk secondChunk;

  //assigns the values to the new split chunk
  secondChunk.memoryAddress = secondChunkPtr;
  secondChunk.chunkSize = chunk_size;
  secondChunk.beingUsed = true;

  //places the new chunk into the free list before the chunk it split from
  list<Chunk>::iterator retVal = freeList.insert(chunk, secondChunk);

  //updates the new size of the split appart chunk
  chunk->chunkSize = chunk->chunkSize - chunk_size;
  chunk->beingUsed = false;

  return retVal;
}

//print free list formatted
void printFree(){
  cout << "Free Memory" << endl;
  printf("Index\t|Address\t|Size\n");
  int number = 0;
  for (list<Chunk>::iterator currChunk = freeList.begin();
       currChunk != freeList.end(); currChunk++){
    printf("%d\t|%p\t|%d\n",
      number,
      currChunk->memoryAddress,
      currChunk->chunkSize);
    number++;
  }
  cout << endl;
}

//print allocated list formatted
void printAllocated(){
  cout << "Allocated Memory" << endl;
  printf("Index\t|Address\t|Size\t|Memory\n");
  int number = 0;
  for (list<Chunk>::iterator currChunk = allocatedList.begin();
       currChunk != allocatedList.end(); currChunk++){
    char* word = (char*)currChunk->memoryAddress;
    printf("%d\t|%p\t|%d\t|%s\n",
      number,
      currChunk->memoryAddress,
      currChunk->chunkSize,
      word);
    number++;
  }
  cout << endl;
}

//print both lists formatted
void printLists(){
  cout << "*******************************************" << endl;
  printFree();
  cout << "-------------------------------------------" << endl;
  printAllocated();
  cout << "*******************************************" << endl << endl;
}

void setVersion(int versionPass){
  version = versionPass;
}

int getVersion(){
  return version;
}

void printListSizes(){
  cout << "*****************LIST SIZES*****************" << endl
       << "AllocList: " << allocatedList.size() << endl;
  cout << "-------------------------------------------" << endl
       << "FreeList: " << freeList.size() << endl;
  cout << "*******************************************" << endl << endl;
}
