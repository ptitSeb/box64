#ifndef _GLOBAL_SYMBOLS_H_
#define _GLOBAL_SYMBOLS_H_

void addGlobalRef(void* p, const char* symname);

//GTK stuff
void my_checkGlobalGdkDisplay(void);
void my_setGlobalGThreadsInit(void);
void addGlobalGdkDisplayRef(void* p);

//void** my_GetGTKDisplay(void);
void** my_GetGthreadsGotInitialized(void);  // defined in wrappedgthread2

// NCurse / TInfo
void my_checkGlobalTInfo(void);
void my_updateGlobalTInfo(void);

// getopt
void my_checkGlobalOpt(void);
void my_updateGlobalOpt(void);

#ifdef BOX32
// NCurse / TInfo
void my32_checkGlobalTInfo(void);
void my32_updateGlobalTInfo(void);

// getopt
void my32_checkGlobalOpt(void);
void my32_updateGlobalOpt(void);
#endif

// libxslt
void my_checkGlobalXslt(void);
void my_updateGlobalXslt(void);

#endif //_GLOBAL_SYMBOLS_H_