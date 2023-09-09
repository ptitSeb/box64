#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "fileutils.h"

void pressure_vessel(int argc, const char** argv, int nextarg)
{
    // skip all the parameter, but parse some of them
    const char* runtime = getenv("PRESSURE_VESSEL_RUNTIME");
    int ld_lib_path = 0;
    // look for the comand first
    const char* cmd = argv[nextarg];
    int i = 0;
    while(cmd[0]=='-' && cmd[1]=='-') cmd=argv[nextarg+(++i)];
    int is_usr = (cmd && strlen(cmd)>5 && strstr(cmd, "/usr/")==cmd)?1:0;
    if(argv[nextarg][0]=='-' && argv[nextarg][1]=='-')
        while(argv[nextarg][0]=='-' && argv[nextarg][1]=='-') {
            if(strstr(argv[nextarg], "--env-if-host=PRESSURE_VESSEL_APP_LD_LIBRARY_PATH=")==argv[nextarg]) {
                if(is_usr) {
                    // transform RESSURE_VESSEL_APP_LD_LIBRARY_PATH to BOX86_ / BOX64_ LD_LIBRARY_PATH
                    char tmp[strlen(argv[nextarg])+(runtime?(strlen(runtime)+1):0)];
                    strcpy(tmp, "");
                    strcat(tmp, argv[nextarg]+strlen("--env-if-host=PRESSURE_VESSEL_APP_"));
                    char *p = strchr(tmp, '=');
                    *p ='\0'; ++p;
                    if(runtime) {
                        memmove(p+strlen(runtime), p, strlen(p));
                        memmove(p, runtime, strlen(runtime));
                        p[strlen(runtime)]= ':';
                    }
                    ld_lib_path = 1;
                    setenv(tmp, p, 1);
                    printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                }
            } else if(strstr(argv[nextarg], "--env-if-host=STEAM_RUNTIME_LIBRARY_PATH=")==argv[nextarg]) {
                if(is_usr) {
                    // transform RESSURE_VESSEL_APP_LD_LIBRARY_PATH to BOX86_ / BOX64_ LD_LIBRARY_PATH
                    char tmp[strlen(argv[nextarg])+150];
                    strcpy(tmp, "BOX86_LD_LIBRARY_PATH=/lib/box86:/usr/lib/box86:/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu:/usr/lib/i686-pc-linux-gnu:/usr/lib32:");
                    strcat(tmp, argv[nextarg]+strlen("--env-if-host=STEAM_RUNTIME_LIBRARY_PATH="));
                    char *p = strchr(tmp, '=');
                    *p ='\0'; ++p;
                    setenv(tmp, p, 1);
                    printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                    strcpy(tmp, "BOX64_LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:");
                    strcat(tmp, argv[nextarg]+strlen("--env-if-host=STEAM_RUNTIME_LIBRARY_PATH="));
                    p = strchr(tmp, '=');
                    *p ='\0'; ++p;
                    setenv(tmp, p, 1);
                    printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                }
            } else if(!strcmp(argv[nextarg], "--")) {
                printf_log(LOG_DEBUG, "End of pressure-vessel-wrap parameters\n");
            }else {
                printf_log(LOG_DEBUG, "Ignored parameter: \"%s\"\n", argv[nextarg]);
            }
            ++nextarg;
        }
    if(runtime && !ld_lib_path) {
        setenv("LD_LIBRARY_PATH", runtime, 1);
        printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", "LD_LIBRARY_PATH", runtime);
    }
    printf_log(LOG_DEBUG, "Ready to launch \"%s\", nextarg=%d, argc=%d\n", argv[nextarg], nextarg, argc);
    const char* prog = argv[nextarg];
    my_context = NewBox64Context(argc - nextarg);
    int x86 = my_context->box86path?FileIsX86ELF(argv[nextarg]):0;
    int x64 = my_context->box64path?FileIsX64ELF(argv[nextarg]):0;
    // create the new argv array
    const char** newargv = (const char**)box_calloc((argc-nextarg)+1+((x86 || x64)?1:0), sizeof(char*));
    if(x86 || x64) {
        newargv[0] = x64?my_context->box64path:my_context->box86path;
        printf_log(LOG_DEBUG, "argv[%d]=\"%s\"\n", 0, newargv[0]);    
        for(int i=nextarg; i<argc; ++i) {
            printf_log(LOG_DEBUG, "argv[%d]=\"%s\"\n", 1+i-nextarg, argv[i]);    
            newargv[1+i-nextarg] = argv[i];
        }
    } else {
        for(int i=nextarg; i<argc; ++i) {
            printf_log(LOG_DEBUG, "argv[%d]=\"%s\"\n", i-nextarg, argv[i]);    
            newargv[i-nextarg] = argv[i];
        }
    }
    //setenv("BOX64_PREFER_EMULATED", "1", 1);
    //setenv("BOX86_PREFER_EMULATED", "1", 1);

//setenv("BOX64_TRACE_FILE", "/home/seb/trace64-%pid.txt", 1);
//setenv("BOX86_TRACE_FILE", "/home/seb/trace86-%pid.txt", 1);
//setenv("BOX86_LOG", "1", 1);
//setenv("BOX64_LOG", "1", 1);
//setenv("BOX86_SHOWSEGV", "1", 1);
//setenv("BOX64_SHOWSEGV", "1", 1);
//setenv("BOX64_DYNAREC_LOG", "1", 1);

    printf_log(LOG_DEBUG, "Run %s %s and wait\n", x86?"i386":(x64?"x86_64":""), argv[nextarg]);
    pid_t v = vfork();
    if(v==-1) {
        printf_log(LOG_NONE, "vfork failed!\n");
        FreeBox64Context(&my_context);
        return;
    }
    if(!v) {
        // child process, exec the program...
        int ret = execvp(newargv[0], (char * const*)newargv);
        printf_log(LOG_DEBUG, "Error launching program: %d (%s)\n", ret, strerror(errno));
        exit(0);
    } else {
        // parent process, wait the end of child
        FreeBox64Context(&my_context);
        int wstatus;
        wait(&wstatus);
        //waitpid(v, &wstatus, 0);
        exit(0);
    }
}