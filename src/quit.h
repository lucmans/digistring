
#ifndef QUIT_H
#define QUIT_H


void signal_handler(const int signum);

bool poll_quit();
void set_quit();
void reset_quit();


#endif  // QUIT_H
