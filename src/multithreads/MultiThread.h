
#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include "../db/Database.h"
#include <pthread.h>
#include <thread>

void ThreadNum_Detection();

void set_ThreadNum(unsigned int temp);

unsigned int get_ThreadNum();

#endif
