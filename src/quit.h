#ifndef DIGISTRING_QUIT_H
#define DIGISTRING_QUIT_H


void signal_handler(const int signum);

bool poll_quit();
void set_quit();
void reset_quit();


#endif  // DIGISTRING_QUIT_H
