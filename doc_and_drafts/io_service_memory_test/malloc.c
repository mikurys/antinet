#define _GNU_SOURCE
//#include<iostream>
#include <stdio.h>
#include <dlfcn.h>

static void* (*real_malloc)(size_t)=NULL;

static size_t all_size;
static void mtrace_init(void)
{
	real_malloc = (void*(*)(size_t))dlsym(RTLD_NEXT, "malloc");
	if (NULL == real_malloc) {
		fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
	}
}

void *malloc(size_t size)
{
	if(real_malloc==NULL) {
		mtrace_init();
	}

	void *p = NULL;
	all_size += size;
	printf("allocated size = %d, all = %d\n", size, all_size);
	p = real_malloc(size);
	return p;
}
/*
int main()
{
	int *ptr = (int*) malloc(10);
	int *tab = new int[20];

	//free(ptr);
}
*/
