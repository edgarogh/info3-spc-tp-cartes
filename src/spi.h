// Copyright © 2022 Edgar Onghena <dev@edgar.bzh>
// Ce fichier est sous license APACHE-2.0
// http://www.apache.org/licenses/LICENSE-2.0

#ifndef _SPI_H_
#define _SPI_H_

#include <stdbool.h>
#include <stdint.h>

void spiBegin();
void spiSetSs(bool high);
void spiTransfer16(uint8_t octet0, uint8_t octet1);

#endif
