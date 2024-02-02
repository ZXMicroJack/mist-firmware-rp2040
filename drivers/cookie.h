#ifndef _COOKIE_H
#define _COOKIE_H

void cookie_Set();
void cookie_Reset();
uint8_t cookie_IsPresent();

void cookie_Set2(uint8_t data);
uint8_t cookie_IsPresent2();

#ifdef USE_COOKIE3
uint8_t cookie_IsPresent3();
#endif

#endif
