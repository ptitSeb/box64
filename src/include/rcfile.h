#ifndef __RCFILE_H__
#define __RCFILE_H__

void LoadRCFile(const char* filename);
void DeleteParams(void);
void ApplyParams(const char* name);
const char* GetLastApplyName();

#endif //__RCFILE_H__