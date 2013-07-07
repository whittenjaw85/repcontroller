#ifndef _GCODE_PROCESS_H
#define _GCODE_PROCESS_H

#include "gcode_parse.h"

//current tool
extern uint8_t tool;

//changed tool
extern uint8_t next_tool;

//process line of gcode
void process_gcode_command(void);
#endif // GOCDE_PROCESS_H
