#ifndef __DICTIONNARY_H_
#define __DICTIONNARY_H_
#include <stdint.h>

typedef void dic_t;

dic_t *NewDictionnary(void);
void FreeDictionnary(dic_t **dic);

const char* AddDictionnary(dic_t* dic, const char* s);
int ExistDictionnary(dic_t* dic, const char* s);

#endif //__DICTIONNARY_H_