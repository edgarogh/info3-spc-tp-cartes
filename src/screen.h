// Copyright © 2022 Edgar Onghena <dev@edgar.bzh>
// Ce fichier est sous license APACHE-2.0
// http://www.apache.org/licenses/LICENSE-2.0

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <stdint.h>

void screen_init(void);
void screen_writeCounter(uint32_t tenths);
void screen_setBrightness(unsigned char brightness);

#endif
