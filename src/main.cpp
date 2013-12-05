#include "scuttle.hpp"
#include <algorithm>
#include <sstream>
#include <uv.h>

using namespace std;

ScuttleButt sc;
map<string,Message> store; // Just a default, in-memory, store...

uv_stream_t * stream;

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
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
  memset(buf->base, 0, suggested_size);
}

// Read the data from the server and parse it.
void parse_read(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, NULL);
  } else if (nread > 0) {
    stringstream ss(buf->base);
    while(ss.peek() != EOF) {
      sc.getMessage(ss,applyUpdate);
    }
  }
  free(buf->base);
}

void on_connection(uv_connect_t * req, int status) {
  if (status < 0) return;
  // setup the buffer containing the digest message...
  string out = sc.getDigest() + "\n";
  char * outstr = new char[out.length()+1];
  memset(outstr, 0, out.length()+1);
  out.copy(outstr, out.length()+1);
  uv_buf_t outbuf = uv_buf_init(outstr, out.length());
  
  //setup the read callback, and send out digest message...
  stream = req->handle;
  uv_write_t write_req;
  
  uv_read_start((uv_stream_t *)stream, alloc_buffer, parse_read);
  uv_write(&write_req, stream, &outbuf, 1, NULL);
  delete[] outstr;
}

void on_sync() {
  cout << "Closing connection and printing out store contents:" << endl << endl;
  uv_close((uv_handle_t *) stream, NULL);
  for (map<string,Message>::iterator it = store.begin(); it != store.end(); ++it) {
    cout << it->second.value << endl;
  }
}

int main() {  
  sc.setSyncCallback(on_sync);
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
