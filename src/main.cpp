#include "scuttle.hpp"
#include <algorithm>
#include <uv.h>

using namespace std;


ScuttleButt sc;
map<string,Message> store; // Just a default, in-memory, store...

void applyUpdate(const string & key, const Message & m) {
  if (key == "__proto__") return;
  map<string,Message>::iterator it = store.find(key);
  if ( (it != store.end()) && (it->second.version > m.version) ) {
    //This is old data...
    return;
  }
  store[key] = m;
  return;
}

void alloc_buffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
  *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

// This just echoes stuff to the command line...
void echo_read(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, NULL);
  } else if (nread > 0) {
    if (memcmp("\"SYNC\"", buf->base, 6) == 0) {
      uv_close((uv_handle_t *) stream, NULL);
    }
    cout.write(buf->base, nread);
  }
  if (buf->base)
    free(buf->base);
}

void on_connection(uv_connect_t * req, int status) {
  if (status < 0) return;
  // setup the buffer containing the digest message...
  string out = sc.getDigest() + "\n";
  char * outstr = new char[out.length()+1];
  out.copy(outstr, out.length()+1);
  uv_buf_t outbuf = uv_buf_init(outstr, out.length());
  
  //setup the read callback, and send out digest message...
  uv_stream_t * stream = req->handle;
  uv_write_t write_req;
  
  uv_read_start((uv_stream_t *)stream, alloc_buffer, echo_read);
  uv_write(&write_req, stream, &outbuf, 1, NULL);
  delete[] outstr;
}

int main() {
  
  string id = sc.createID();
  string out = sc.getDigest();
  
  cout << out << endl;
  
  uv_tcp_t socket;
  uv_tcp_init(uv_default_loop(), &socket);
  uv_connect_t connect;
  
  struct sockaddr_in dest_addr;
  uv_ip4_addr("127.0.0.1", 8124, &dest_addr);
  uv_tcp_connect(&connect, &socket, (sockaddr *)&dest_addr, on_connection);
  
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  
}