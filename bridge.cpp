bool send_cdc(const char*, int);
int read_cdc(char*&);
bool uart_tx(const char*, int);

class Buffer {
public:
  Buffer(void): index(0) {}
  bool write(char c) { buffer[index++] = c; return index == 64; }
  void send(void) { if (send_cdc(buffer, index)) index = 0; }
  bool empty(void) { return !index; }
private:
  char buffer[64];
  int index;
};

namespace
{

Buffer buffer[2];
int index, count;

} // anonymous

void uart_rx(char c) {
  if (buffer[index].write(c)) { index ^= 1; count = 0; }
}

void bridge_poll(unsigned t) {
  if (uart_tx(0,0)) {
    int len;
    char *buf;
    if ((len = read_cdc(buf)))
      uart_tx(buf, len);
  }
  buffer[index ^ 1].send();
  if (buffer[index].empty()) count = 0;
  if (count++ > 5) {
    buffer[index].send();
    index ^= 1; count = 0;
  }
}
