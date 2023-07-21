#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "dictionnary.h"
#include "custommem.h"
#include "khash.h"

KHASH_SET_INIT_STR(dic);

dic_t *NewDictionnary()
{
    dic_t *dic = kh_init(dic);
    return dic;
}

void FreeDictionnary(dic_t **d)
{
    if(!d || !*d)
        return;
    kh_dic_t* dic = (kh_dic_t*)*d;
    const char* k;
    kh_foreach_key(dic, k, box_free((void*)k));
    kh_destroy(dic, dic);
    *d = NULL;
}

const char* AddDictionnary(dic_t* d, const char* s)
{
    kh_dic_t* dic = (kh_dic_t*)d;
    khint_t k = kh_get(dic, dic, s);
    if(k!=kh_end(dic))
        return kh_key(dic, k);
    char* n = box_strdup(s);
    int ret;
    k = kh_put(dic, dic, n, &ret);
    return n;
}

int ExistDictionnary(dic_t* d, const char* s)
{
    kh_dic_t* dic = (kh_dic_t*)d;
    khint_t k = kh_get(dic, dic, s);
    if(k!=kh_end(dic))
        return 1;
    return 0;

}