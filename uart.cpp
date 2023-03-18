#include <xc.h>
#include "uart.h"

void uart_rx(char);

namespace
{

const char *buffer;
int length;

} //anonymous

bool stop_selection(int i) {
    if (i == NUM_STOP_BITS_1) { U1MODEbits.STSEL = 0; return true; }
    if (i == NUM_STOP_BITS_2) { U1MODEbits.STSEL = 1; return true; }
    return false;
}

bool parity_data_selection(int p, int d) {
    if ((p == PARITY_NONE) && (d == 9)) { U1MODEbits.PDSEL = 3; return true; }
    if (d == 8) {
        if (p == PARITY_NONE) { U1MODEbits.PDSEL = 0; return true; }
        if (p == PARITY_EVEN) { U1MODEbits.PDSEL = 1; return true; }
        if (p == PARITY_ODD) { U1MODEbits.PDSEL = 2; return true; }
    }
    return false;
}

bool baud_selection(unsigned b) {
    unsigned baud = (2500000 + (b>>1))/b - 1;
    if (baud && (baud < 65536)) return U1BRG = baud;
    return false;
}

bool uart_tx(const char *s, int len) {
  if (IEC1bits.U1TXIE) return false;
  if (len) {
    buffer = s;
    length = len;
    IEC1bits.U1TXIE = 1;
  }
  return true;
}

extern "C"
void __attribute__((interrupt(ipl1soft), vector(_UART1_VECTOR), nomips16)) u1ISR(void) {
  static int index;
  if (IFS1bits.U1RXIF) {
    uart_rx(U1RXREG);
    IFS1bits.U1RXIF = 0;
  }
  if ((IEC1bits.U1TXIE) && (IFS1bits.U1TXIF)) {
    U1TXREG = buffer[index++];
    if (index == length) {
      index = 0;
      IEC1bits.U1TXIE = 0;
    }
    IFS1bits.U1TXIF = 0;
  }
}

