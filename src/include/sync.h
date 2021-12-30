#pragma once 

void spin_lock(int volatile* p);
void spin_unlock(int volatile* p);
void spin_wait(int volatile *p, int val);