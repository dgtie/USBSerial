void init(void), toggle_LED(void);
bool wait(unsigned);    // always return true
void USBDeviceInit(void);

int main(void) {
  init();
  USBDeviceInit();
  while (wait(0));      // call wait() to transfer control to poll()
}

void read_switch(void);
void poll(unsigned t) {
  if (!(t & 15)) read_switch();		// read switch every 15 ms
}

void on_switch_change(bool b) { if (!b) toggle_LED(); }

