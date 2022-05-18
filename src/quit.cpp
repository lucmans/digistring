#include "quit.h"

#include "error.h"

#include <csignal>  // catching signals
#include <cstring>  // sigabbrev_np()
#include <execinfo.h>  // backtrace() functions


static volatile bool quit = false;

bool poll_quit() {
    return quit;
}

void set_quit() {
    info("Quitting application on next cycle...");
    quit = true;
}

void reset_quit() {
    quit = false;
}


void set_signal_handlers() {
    signal(SIGINT, quit_signal_handler);
    signal(SIGTERM, quit_signal_handler);

    signal(SIGSEGV, backtrace_quit_signal_handler);
}

void quit_signal_handler(const int signum) {
    if(quit) {
        info("Received signal 'SIG" + STR(sigabbrev_np(signum)) + "' while quitting; will now force quit");
        exit(-2);
    }

    __msg("");  // Print on a new line for when using ctrl+c to send a SIGINT (causes ^C to be printed)
    info("Signal 'SIG" + STR(sigabbrev_np(signum)) + "' received");

    set_quit();
}


void backtrace_quit_signal_handler(const int signum) {
    if(signum == SIGSEGV)
        error("Segmentation fault occurred; printing stack trace (use addr2line to resolve offsets)...");
    else
        error("Signal 'SIG" + STR(sigabbrev_np(signum)) + "' received; printing stack trace (use addr2line to resolve offsets)...");
    hint("First two in call stack are likely from signal handler");

    const int n_frames = 10;
    void *frames[n_frames];
    int size;
    size = backtrace(frames, n_frames);
    // for(int i = 0; i < n_frames; i++)
    //     std::cout << frames[i] << std::endl;

    char **strings = backtrace_symbols(frames, size);
    if(strings != NULL) {
        for(int i = 0; i < size; i++)
            std::cout << i + 1 << ": " << strings[i] << std::endl;
    }

    // TODO: Use dladdr1() and/or abi::__cxa_demangle() for more information

    free(strings);
    exit(EXIT_FAILURE);
}


/* Prints file+line */
// #include <execinfo.h>
// #include <link.h>
// #include <stdlib.h>
// #include <stdio.h>
// // converts a function's address in memory to its VMA address in the executable file. VMA is what addr2line expects
// size_t ConvertToVMA(size_t addr)
// {
//   Dl_info info;
//   link_map* link_map;
//   dladdr1((void*)addr,&info,(void**)&link_map,RTLD_DL_LINKMAP);
//   return addr-link_map->l_addr;
// }
// void backtrace_quit_signal_handler(const int signum)
// {
//     static bool called = false;
//     if(called)
//         abort();
//     called = true;

//   void *callstack[128];
//   int frame_count = backtrace(callstack, sizeof(callstack)/sizeof(callstack[0]));
//   for (int i = 0; i < frame_count; i++)
//   {
//     char location[1024];
//     Dl_info info;
//     if(dladdr(callstack[i],&info))
//     {
//       char command[256];
//       size_t VMA_addr=ConvertToVMA((size_t)callstack[i]);
//       //if(i!=crash_depth)
//         VMA_addr-=1;    // https://stackoverflow.com/questions/11579509/wrong-line-numbers-from-addr2line/63841497#63841497
//       snprintf(command,sizeof(command),"addr2line -e %s -Ci %zx",info.dli_fname,VMA_addr);
//       system(command);
//     }
//   }
// }
