#ifndef VI_H
#define VI_H

#include <windows.h>
#include <stdbool.h>

void vi_enter_normal(void);
void vi_enter_insert(void);
bool vi_process_key(WORD vk, bool keydown);

#endif // VI_H