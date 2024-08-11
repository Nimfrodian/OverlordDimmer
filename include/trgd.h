/*
*
*   Trigger driver for AC mosfet triggering
*
*/

#ifndef TRGD_H
#define TRGD_H

#include "espa.h"
#include "tmra.h"
#include "lgic.h"

void trgd_init(void);
void trgd_run(void);

#endif