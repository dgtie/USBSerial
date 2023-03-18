#include <xc.h>

const int devcfg3 __attribute__((section(".config_BFC00BF0"), used)) = 0x0FFFFFFF;
const int devcfg2 __attribute__((section(".config_BFC00BF4"), used)) = 0xFFF979D9;
const int devcfg1 __attribute__((section(".config_BFC00BF8"), used)) = 0xFF74CDDB;
const int devcfg0 __attribute__((section(".config_BFC00BFC"), used)) = 0x7FFFFFFB;

#define MS 20000        // 1 ms

void bridge_poll(unsigned);

void init(void) {
  __builtin_disable_interrupts();        // global interrupt enable
  SYSKEY = 0;                   // ensure OSCCON is locked
  SYSKEY = 0xAA996655;          // unlock sequence
  SYSKEY = 0x556699AA;
  CFGCONbits.IOLOCK = 0;        // allow write
  U1RXR = 0b0100;               // PB2
  RPB3R = 0b0001;               // U1TX
  CFGCONbits.IOLOCK = 1;        // forbidden write
  SYSKEY = 0;                   // relock
  ANSELBCLR = 0xc;		// set uart pins digital
  U1BRG = 21;			// baud rate = 115200
  U1STASET = 0x1400;		// URXEN, UTXEN
  IPC8bits.U1IP = 1;
  IEC1bits.U1RXIE = 1;
  U1MODEbits.ON = 1;
  CNPUBSET = 1 << 8;		// enable B8 pull-up resistor
  TRISBCLR = 1 << 9;            // clear tris -> set B9 as output
  _CP0_SET_COMPARE(MS);         // set core timer interrupt condition
  IPC0bits.CTIP = 1;            // core timer interrupt at lowest priority
  IEC0bits.CTIE = 1;            // enable core timer interrupt
  INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();        // global interrupt enable
}

void toggle_LED(void) { LATBINV = 1 << 9; }

static volatile unsigned tick;

void poll(unsigned timestamp);  // it will be called when timestamp changes

static void loop(unsigned t) {
  static unsigned tick;
  if (tick != t) poll(tick = t);
}

bool wait(unsigned t) {
  unsigned u = tick;
  for (loop(u); t--; u = tick) while (u == tick) loop(u);
  return true;
}

extern "C"
__attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16))
void ctisr(void) {
  _CP0_SET_COMPARE(_CP0_GET_COMPARE() + MS);    // next interrupt at 1 ms
  bridge_poll(tick++);                          // keep track of time
  IFS0bits.CTIF = 0;                            // clear flag
}

void on_switch_change(bool);
void read_switch(void) {
  static int state = 0x100;	// assume switch is not pressed
  if (state != (PORTB & 0x100)) on_switch_change(state ^= 0x100);
}

