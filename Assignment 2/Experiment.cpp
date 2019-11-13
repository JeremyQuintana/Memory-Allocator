#include "Experiment.h"

static int maxThreads;
string getRandomWord();
void iteration(string words[ASSIGNS]);

/*alloc and dealloc runs on the same thread to allow essentially test
concurrency with multiple allocs, multiple deallocs, and multple allocs
and deallocs together
Essentially tests, mimicing real world use of allocators as process may alloc
or dealloc at any time*/
void* allocDealloc(void* args){
  string* word = static_cast<string*>(args);

  void* address = alloc(word->size());

  // gets the char pointer to the start of memeory
  char* wordPtr = (char*)(address);

  //iterates through the char pointer to get the whole word
  for (int i = 0; i < (int)word->size(); i++){
    ((char*)wordPtr)[i] = (*word)[i];
  }

  dealloc(address);

  return nullptr;
}

void test(int loops){

  //get all words from file
  string words[loops][ASSIGNS];
  for (int i = 0; i < loops; i++){
    for (int x = 0; x < ASSIGNS; x++){
      words[i][x] = getRandomWord();
    }
  }

  //START TEST
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < loops; i++){
    iteration(words[i]);
  }

  auto end = std::chrono::steady_clock::now();
  //END TEST

  //print details
  cout << "Final Lists:" << endl;
  printLists();

  cout << "Final Sizes:" << endl;
  printListSizes();

  cout <<
  "Experiment Multi Threaded Duration: " << endl <<
  std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() <<
  " ns" << endl << endl;
}

void iteration(string words[ASSIGNS]){
  //Perform all allocations
  //Stays in loop until all allocations are performed
  int currAssign = 0;
  // std::cout << "ALLOCATING" << std::endl;
  while (currAssign < ASSIGNS){
    //Continously creates threads to perform allocations
    /*Once max amount of threads is reached or number of desired allocations
    are reached then it exits*/
    pthread_t threads[maxThreads];
    int threadsCreated = 0;
    while (threadsCreated < maxThreads && currAssign < ASSIGNS){
      //runs alloc and dealloc on different thread
      pthread_create(&threads[threadsCreated], NULL, allocDealloc, &words[currAssign]);

      ++currAssign;
      ++threadsCreated;
    }

    /*When finished allocating or max amount of threads is reached, this loops
    through and joins all threads*/
    int threadsJoined = 0;
    while (threadsJoined < threadsCreated){
      pthread_join(threads[threadsJoined], NULL);

      ++threadsJoined;
    }

    //Loops creation of threads and joining until all allocations are performed
  }
}

string getRandomWord(){
  ifstream inFile;

  //random device to get random line
  random_device rd;
  uniform_int_distribution<int> range(0, MAX_LINES);
  int lineNumber = range(rd);

  //open up data full of names
  //use of data set from https://github.com/dominictarr/random-name
  inFile.open("names.txt");

  //iterates through file until the line which was randomly selected is found
  string line;
  for (int i = 0; i < lineNumber && !inFile.eof() && inFile.good(); i++){
    getline(inFile, line);
  }

  //adds null character string to end to mimic c string usage in later conversion
  line += '\0';

  inFile.close();

  return line;
}

void setMaxThreads(int threads){
  maxThreads = threads;
}
