#ifndef STDLIB_H
#define STDLIB_H

#include "../../include/stdlib.h"

int __putenv(char *, size_t, char *);
void __env_rm_add(char *, char *);
int __mkostemps(char *, int, int);
int __ptsname_r(int, char *, size_t);
char *__randname(char *);

#endif
