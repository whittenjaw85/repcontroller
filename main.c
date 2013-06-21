#include <avr/io.h>

int main(void)
{
    for(;;)
    {
        if((serial_rxchars() != 0) && (queue_full() == 0)){
            uint8_t c = serial_popchar();
            //gcode_parse_char(c);
        }

    }
}
