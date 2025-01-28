#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
void clr_flags();  
void enable_watchdog();
void disable_watchdog();
void reset_watchdog();
void init_watchdog(unsigned char interval);
#endif
