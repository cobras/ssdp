#ifndef __COMMON_H__
#define __COMMON_H__

#define log(...) fprintf(stderr, __VA_ARGS__)
//#define log(...)
#define BLOC_LEN 1024
#define TRIM_R(ptr) while(isspace(*(ptr))) ptr++;

#endif
