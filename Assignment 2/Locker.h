#include <pthread.h>
#include <iostream>

class Locker{
private:
  std::string DataStructure;

  int readLockCounter;
  int writeQueueCounter;
  bool writing;

  pthread_cond_t triggerWrite;
  pthread_cond_t triggerRead;

  pthread_mutex_t lock;


public:
  Locker(std::string dataStructue);
  void readLock();
  void readUnlock();
  void writeLock();
  void writeUnlock();
};
