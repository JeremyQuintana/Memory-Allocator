#include "Experiment.h"
#include <unistd.h>

#define EXIT_SUCCESS 0

using std::cin;

int main(int argc, char* argv[]){
  bool validated = true;
  int iterations = 0;
  int version = 0;
  int threads = 0;

  //checks amout of arguments is correct
  if (argc == 4){
    //validation for version input
    //attempts to assign version
    try{
      version = std::stoi(argv[1]);
      //checks for valid version
      if (version == 1 || version == 2 || version == 3){
        setVersion(version);
      } else{
        throw nullptr;
      }
      //if error then its an invalid version
    } catch(...){
      cout << "Error - Invalid Version" << endl;
      validated = false;
    }

    //validation for iteration input
    //attempts to assign iteration
    try{
      iterations = std::stoi(argv[2]);
      if (iterations < 0){
        throw nullptr;
      }
    } catch(...){
      cout << "Error - Invalid Iterations" << endl;
      validated = false;
    }

    //validation for thread input
    try{
      threads = std::stoi(argv[3]);
      if (threads <= 0){
        throw nullptr;
      } else {
        setMaxThreads(threads);
      }
    } catch(...){
      cout << "Error - Invalid Threads" << endl;
      validated = false;
    }
  //if wrong amount of parameters
} else{
    cout << "Error - Wrong Number of Paramenters" << endl;
    validated = false;
  }

  //if passes all validation
  if (validated == true){
    //iterate through a cycle of tests a given amount of times
    test(iterations);
    // if (execv("../Assignment 1/run", argv) == EOF) {
    //     cout << "Failed to run single threaded version of program" << endl;
    // }
  } else{
    cout << endl <<
    "TO RUN:" << endl <<
    "./run <strategy> <testiterations> <threads>" << endl <<
    endl <<
    "Strategy Options:" << endl <<
    "First Fit = 1" << endl <<
    "Best Fit = 2" << endl <<
    "Worst Fit = 3" << endl <<
    endl <<
    "Iterations:" << endl <<
    "Number or iterations of allocating 10 random names and deallocating them" << endl <<
    endl <<
    "Threads" << endl <<
    "Maximum amount of threads to run at one time"
    << endl;
  }

  return EXIT_SUCCESS;
}
