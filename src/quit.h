#ifndef DIGISTRING_QUIT_H
#define DIGISTRING_QUIT_H


bool poll_quit();
void set_quit();
void reset_quit();


void set_signal_handlers();

void quit_signal_handler(const int signum);
void backtrace_quit_signal_handler(const int signum);


#endif  // DIGISTRING_QUIT_H
