#ifndef _GLOBAL_SYMBOLS_H_
#define _GLOBAL_SYMBOLS_H_

//GTK stuff
void my_checkGlobalGdkDisplay(void);
void my_setGlobalGThreadsInit(void);

//void** my_GetGTKDisplay(void);
void** my_GetGthreadsGotInitialized(void);  // defined in wrappedgthread2

// NCurse / TInfo
void my_checkGlobalTInfo(void);
void my_updateGlobalTInfo(void);

// getopt
void my_checkGlobalOpt(void);
void my_updateGlobalOpt(void);

#endif //_GLOBAL_SYMBOLS_H_