#include "dev/uart1.h"
#include "dev/serial-line.h"

/* serial line byte input handler interface (dev/uart1.h)
 * default handler = serial_line_input_byte (Contiki's default behaviour)
 */
uart_input_handler_t uart1_input_handler;
void uart1_set_input(uart_input_handler_t input)
{
    uart1_input_handler = input;
}
