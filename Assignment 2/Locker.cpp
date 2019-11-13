#include "Locker.h"

Locker::Locker(std::string dataStructure){
  DataStructure = dataStructure;
  readLockCounter = 0;
  writeQueueCounter = 0;
  writing = false;
}

/*
HAVING ONE MUTEX LOCK AT THE START AND END OF THE FUNCTION ENSURES ONLY ONE
FUNCTION CAN BE RUN AT A TIME ESSENTIALLY REMOVING DEADLOCKS IN THE COUNTERS

EACH READ OR WRITE LOCK NEEDS TO BE FOLLOWED BY THE RELATIVE UNLOCK TO PREVENT
DEADLOCKS
*/

/*
Readlocks allow multiple threads to read if no writers are in queue
It also increments a counter of amount of readers to tell the program
if theres anything curently reading and tells the reader behind it in queue to
also run (recursive)
*/
void Locker::readLock(){
  pthread_mutex_lock(&lock);
  while (writeQueueCounter != 0){
    pthread_cond_wait(&triggerRead, &lock);
  }
  pthread_cond_signal(&triggerRead);

  ++readLockCounter;
  pthread_mutex_unlock(&lock);
}

/*
ReadUnlock decrements the counter of amount of readers and after each unlock
tells other threads waiting to write to run
NOTE it is up to the writelock function to know if there are no readers left
after getting a signal
*/
void Locker::readUnlock(){
  pthread_mutex_lock(&lock);
  --readLockCounter;
  pthread_cond_signal(&triggerWrite);
  pthread_mutex_unlock(&lock);
}

/*
Writelocks allow only a single thread to write at a time and only if there are
no readers
It increments the amount of writers in queue to indicate there are writers
waiting as there is implemented priority towards writers
If signaled it checks that there are no writers or that there are anything
currently writing
It then decrements the amount waiting in writer queue then indicates that
there is a thread writing
*/
void Locker::writeLock(){
  pthread_mutex_lock(&lock);
  ++writeQueueCounter;

  while (writing == true || readLockCounter != 0){
    pthread_cond_wait(&triggerWrite, &lock);
  }

  --writeQueueCounter;
  writing = true;

  pthread_mutex_unlock(&lock);


}

/*
Indicates that there is none writing anymore
And it then checks if theres any waiting to write if there is then it sends
the next a signal to run
And if theres non then it triggers all the readers that they can run
*/
void Locker::writeUnlock(){
  pthread_mutex_lock(&lock);

  writing = false;

  if (writeQueueCounter != 0){
    pthread_cond_signal(&triggerWrite);
  } else {
    pthread_cond_signal(&triggerRead);
  }
  pthread_mutex_unlock(&lock);
}
