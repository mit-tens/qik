#ifndef COMMON_H
#define COMMON_H

#define nelem(x) (sizeof(x) / sizeof(x[0]))

#define randrng(min, max) (rand() % (max + 1 - min) + min)

#endif
