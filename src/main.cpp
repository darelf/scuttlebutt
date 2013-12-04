#include "scuttle.hpp"
#include <algorithm>
#include <uv.h>

using namespace std;

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

void echo_read(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, NULL);
  } else if (nread > 0) {
    cout.write(buf->base, nread);
  }
  if (buf->base)
    free(buf->base);
}

void on_new_connection(uv_stream_t * server, int status) {
  if (status < 0) return;
  
  uv_tcp_t * client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), client);
  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

int main() {
  ScuttleButt sc;
  
  string id = sc.createID();
  string out = sc.getDigest();
  
  cout << out << endl;
  
  uv_tcp_t server;
  uv_tcp_init(uv_default_loop(), &server);
  
  struct sockaddr_in bind_addr;
  uv_ip4_addr("0.0.0.0", 7000, &bind_addr);
  uv_tcp_bind(&server, (sockaddr *)&bind_addr);
  int r = uv_listen((uv_stream_t *) &server, 128, on_new_connection);
  if (r < 0) {
    cerr << "Listen error " << uv_err_name(r) << endl;
    return 1;
  }
  
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  
}