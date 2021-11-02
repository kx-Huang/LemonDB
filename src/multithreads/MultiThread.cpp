#include "MultiThread.h"

static unsigned int thread_num;

void ThreadNum_Detection(){
	thread_num = std::thread::hardware_concurrency();
	return;
}

unsigned int get_ThreadNum(){
	return thread_num;
}

void set_ThreadNum(unsigned int temp){
	thread_num = temp;
}


