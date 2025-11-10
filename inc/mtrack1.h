#ifndef _MTRACK_H_
#define _MTRACK_H_

#include <stdlib.h>
#include <stddef.h>

#define malloc(size) _mtrack_patch_malloc(__FILE__,__func__,__LINE__, size)
#define calloc(nmemb, size) _mtrack_patch_calloc(__FILE__,__func__,__LINE__, nmemb, size)
#define realloc(ptr, size) _mtrack_patch_realloc(__FILE__,__func__,__LINE__, ptr, size)
#define free(ptr) _mtrack_patch_free(__FILE__,__func__,__LINE__, ptr)

void *_mtrack_patch_malloc(char* file, const char* function, int line, size_t size);

void *_mtrack_patch_calloc(char* file, const char* function, int line, size_t nmemb, size_t size);

void *_mtrack_patch_realloc(char* file, const char* function, int line, void *ptr, size_t size);

void _mtrack_patch_free(char* file, const char* function, int line, void *ptr);

#endif