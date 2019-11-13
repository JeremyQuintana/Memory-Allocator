#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <pthread.h>
#include "Allocator.h"
#define MAX_LINES 4945
#define ASSIGNS 1000
#define ALLOCARGUMENTS 2

using std::uniform_int_distribution;
using std::random_device;
using std::ifstream;
using std::string;
using std::cout;
using std::endl;

void test(int loops);
void setMaxThreads(int threads);
