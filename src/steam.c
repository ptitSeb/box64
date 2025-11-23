#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <glob.h>

#include "debug.h"
#include "box64context.h"
#include "fileutils.h"

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

void pressure_vessel(int argc, const char** argv, int nextarg, const char* prog)
{
    // skip all the parameter, but parse some of them
    const char* runtime = getenv("PRESSURE_VESSEL_RUNTIME");
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
                    setenv(tmp, p, 1);
                    printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                }
            } else if(strstr(argv[nextarg], "--env-if-host=STEAM_RUNTIME_LIBRARY_PATH=")==argv[nextarg]) {
                if(is_usr) {
                    // transform RESSURE_VESSEL_APP_LD_LIBRARY_PATH to BOX86_ / BOX64_ LD_LIBRARY_PATH
                    char tmp[strlen(argv[nextarg])+150];
                    strcpy(tmp, "BOX86_LD_LIBRARY_PATH=/lib/box86:/usr/lib/box86:/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu:/usr/lib/box86-i386-linux-gnu:/usr/lib/box64-i386-linux-gnu:/usr/lib/i686-pc-linux-gnu:/usr/lib32:");
                    strcat(tmp, argv[nextarg]+strlen("--env-if-host=STEAM_RUNTIME_LIBRARY_PATH="));
                    char *p = strchr(tmp, '=');
                    *p ='\0'; ++p;
                    setenv(tmp, p, 1);
                    printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                    strcpy(tmp, "BOX64_LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:/usr/lib/box64-x86_64-linux-gnu:/usr/lib/box64-i386-linux-gnu:");
                    strcat(tmp, argv[nextarg]+strlen("--env-if-host=STEAM_RUNTIME_LIBRARY_PATH="));
                    p = strchr(tmp, '=');
                    *p ='\0'; ++p;
                    setenv(tmp, p, 1);
                    printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                }
            } else if(strstr(argv[nextarg], "--ld-preloads=")==argv[nextarg]) {
                // transform to BOX86_ / BOX64_ LD_PRELOAD
                char tmp[strlen(argv[nextarg])+4];
                strcpy(tmp, "BOX64_LD_PRELOAD=");
                strcat(tmp, strchr(argv[nextarg], '=')+1);
                char *p = strchr(tmp, '=');
                *p ='\0'; ++p;
                setenv(tmp, p, 1);
                printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
                tmp[3] = '8'; tmp[4] = '6';
                setenv(tmp, p, 1);
                printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", tmp, p);
            } else if(!strcmp(argv[nextarg], "--")) {
                printf_log(LOG_DEBUG, "End of pressure-vessel-wrap parameters\n");
            }else {
                printf_log(LOG_DEBUG, "Ignored parameter: \"%s\"\n", argv[nextarg]);
            }
            ++nextarg;
        }
    if(argv[nextarg] && !strcmp(argv[nextarg], "steamwebhelper")) {
        // just launch it...
        runtime = NULL;
    }
    if(runtime) {
        char sniper[MAX_PATH] = {0};
        // build sniper path
        strcpy(sniper, prog);
        char* p = strrchr(sniper, '/');
        if(p) {
            *p = '\0';
            strcat(sniper, "/../../");
            if(!getenv("BOX64_PYTHON3"))
            {
                // find python3 binary
                glob_t g = {0};
                char tmp[MAX_PATH] = {0};
                snprintf(tmp, sizeof(tmp), "%svar/*/usr/bin/python3", sniper);
                if(!glob(tmp, 0, NULL, &g)) {
                    int found = 0;
                    for(int i=0; i<g.gl_pathc && !found; ++i) {
                        if(FileIsX64ELF(g.gl_pathv[i])) {
                            found = 1;
                            setenv("BOX64_PYTHON3", g.gl_pathv[i], 1);
                            printf_log(LOG_DEBUG, "Found x86_64 python3 binary!");
                        }
                    }
                    globfree(&g);
                }
            }
            strcat(sniper, runtime);
        } else {
            printf_log(LOG_INFO, "Warning, could not guess sniper runtime path\n");
            strcpy(sniper, runtime);    // it's wrong...
        }
        printf_log(LOG_DEBUG, "pressure-vessel sniper env: %s\n", sniper);
        // TODO: read metadata from sniper folder and analyse [Environment] section
        strcat(sniper, "/files");  // this is the sniper root
        // do LD_LIBRARY_PATH
        {
            const char* usrsbinldconfig = "/usr/sbin/ldconfig";
            const char* sbinldconfig = "/sbin/ldconfig";
            const char* ldconfig = "ldconfig";
            const char* ldcmd = ldconfig;
            if(FileExist(usrsbinldconfig, IS_FILE))
                ldcmd = usrsbinldconfig;
            else if(FileExist(sbinldconfig, IS_FILE))
                ldcmd = sbinldconfig;
            char tmp[MAX_PATH*5] = {0};
            // prepare folders, using ldconfig
            snprintf(tmp, sizeof(tmp), "%s -i -n %s/lib/x86_64-linux-gnu", ldcmd, sniper);
            if(system(tmp)<0) printf_log(LOG_INFO, "%s failed\n", tmp);
            snprintf(tmp, sizeof(tmp), "%s -i -n %s/lib/i386-linux-gnu", ldcmd, sniper);
            if(system(tmp)<0) printf_log(LOG_INFO, "%s failed\n", tmp);
            snprintf(tmp, sizeof(tmp), "%s -i -n %s/lib", ldcmd, sniper);
            if(system(tmp)<0) printf_log(LOG_INFO, "%s failed\n", tmp);
            snprintf(tmp, sizeof(tmp), "%s -i -n %s/lib64", ldcmd, sniper);
            if(system(tmp)<0) printf_log(LOG_INFO, "%s failed\n", tmp);
            snprintf(tmp, sizeof(tmp), "%s -i -n %s/lib32", ldcmd, sniper);
            if(system(tmp)<0) printf_log(LOG_INFO, "%s failed\n", tmp);
            // setup LD_LIBRARY_PATH
            const char* ld = getenv("LD_LIBRARY_PATH");
            char tmp2[4096];
            tmp[0] = '\0';
            #define GO(A, B) \
            snprintf(tmp2, sizeof(tmp2), A, B);     \
            if(FileExist(tmp2, 0)) {                \
                strncat(tmp, tmp2, sizeof(tmp)-1);  \
                strncat(tmp, ":", sizeof(tmp)-1);   \
            }
            GO("/usr/lib/%s-x86_64-linux-gnu", "box64")
            GO("/usr/lib/%s-i386-linux-gnu/", "box64")
            GO("%s/lib/x86_64-linux-gnu", sniper)
            GO("%s/lib/i386-linux-gnu", sniper)
            GO("%s/lib/x86_64-linux-gnu/openblas", sniper)
            GO("%s/lib/i386-linux-gnu/openblas", sniper)
            GO("%s/lib/x86_64-linux-gnu/openblas-pthread", sniper)
            GO("%s/lib/i386-linux-gnu/openblas-pthread", sniper)
            GO("%s/lib", sniper)
            GO("%s/lib64", sniper)
            GO("%s/lib32", sniper)
            #undef GO
            if(ld) strncat(tmp, ld, sizeof(tmp)-1);
            setenv("LD_LIBRARY_PATH", tmp, 1);
            printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", "LD_LIBRARY_PATH", tmp);
        }
        // do XDG_DATA_DIRS
        {
            char tmp[MAX_PATH*4] = {0};
            const char* xdg = getenv("XDG_DATA_DIRS");
            snprintf(tmp, sizeof(tmp), "%s/share:%s", sniper, xdg?xdg:"");
            setenv("XDG_DATA_DIRS", tmp, 1);
            printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", "XDG_DATA_DIRS", tmp);
        }
        // disabled GI_TYPELIB_PATH for now
        if(0)
        {
            char tmp[MAX_PATH*4] = {0};
            snprintf(tmp, sizeof(tmp), "%s/lib/x86_64-linux-gnu/girepository-1.0:%s/lib/i386-linux-gnu/girepository-1.0:%s/lib/girepository-1.0", sniper, sniper, sniper);
            setenv("GI_TYPELIB_PATH", tmp, 1);
            printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", "GI_TYPELIB_PATH", tmp);
        }
        // disabled GST_PLUGIN_SYSTEM_PATH
        if(0)
        {
            char tmp[MAX_PATH*4] = {0};
            snprintf(tmp, sizeof(tmp), "%s/lib/x86_64-linux-gnu/gstreamer-1.0:%s/lib/i386-linux-gnu/gstreamer-1.0:%s/lib/gstreamer-1.0", sniper, sniper, sniper);
            setenv("GST_PLUGIN_SYSTEM_PATH", tmp, 1);
            printf_log(LOG_DEBUG, "setenv(%s, %s, 1)\n", "GST_PLUGIN_SYSTEM_PATH", tmp);
        }
        // should disable native gtk on box86 for now, until better wrapping of gstreamer is done on box86 too
        // setenv("BOX86_NOGTK", "1", 1);
        setenv("BOX64_PRESSURE_VESSEL_FILES", sniper, 1);
    }
    printf_log(LOG_DEBUG, "Ready to launch \"%s\", nextarg=%d, argc=%d\n", argv[nextarg], nextarg, argc);
    prog = argv[nextarg];
    my_context = NewBox64Context(argc - nextarg);
    int x86 = my_context->box86path?FileIsX86ELF(argv[nextarg]):0;
    int x64 = my_context->box64path?FileIsX64ELF(argv[nextarg]):0;
    int sh = my_context->bashpath?FileIsShell(argv[nextarg]):0;
    // create the new argv array
    const char** newargv = (const char**)box_calloc((argc-nextarg)+1+((x86 || x64)?1:0), sizeof(char*));
    if(x86 || x64 || sh) {
        newargv[0] = x86?my_context->box86path:my_context->box64path;
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
    //setenv("BOX64_TRACE_FILE", "/tmp/trace64-%pid.txt", 1);
    //setenv("BOX86_TRACE_FILE", "/tmp/trace86-%pid.txt", 1);
    //setenv("BOX86_LOG", "1", 1);
    //setenv("BOX64_LOG", "1", 1);
    //setenv("BOX86_SHOWSEGV", "1", 1);
    //setenv("BOX64_DLSYM_ERROR", "1", 1);
    //setenv("BOX64_SHOWSEGV", "1", 1);
    //setenv("BOX64_SHOWBT", "1", 1);
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
