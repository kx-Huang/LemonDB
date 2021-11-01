
#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include <pthread.h>
#include <thread>
#include "../db/Database.h"


    

void ThreadNum_Detection();

void set_ThreadNum(unsigned int temp);

unsigned int get_ThreadNum();




#endif 
