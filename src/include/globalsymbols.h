#ifndef _GLOBAL_SYMBOLS_H_
#define _GLOBAL_SYMBOLS_H_

//GTK stuff
void my_checkGlobalGdkDisplay();
void my_setGlobalGThreadsInit();

//void** my_GetGTKDisplay();
void** my_GetGthreadsGotInitialized();  // defined in wrappedgthread2

// NCurse / TInfo
void my_checkGlobalTInfo();

// getopt
void my_checkGlobalOpt();

#endif //_GLOBAL_SYMBOLS_H_