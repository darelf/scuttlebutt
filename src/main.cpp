#include "scuttle.hpp"
#include <algorithm>
#include <sstream>
#include <uv.h>

using namespace std;

ScuttleButt sc;
map<string,ScuttleMessage> store; // Just a default, in-memory, store...
map<string,double> remote_digest;

uv_stream_t * stream;

void outputStoreContents() {
  for (map<string,ScuttleMessage>::iterator it = store.begin(); it != store.end(); ++it) {
    cout << it->second.id << ": " << it->second.value << endl;
  }
}

void outputHistoryContents(map<string,double> & source_list) {
  vector<ScuttleMessage> hist = sc.getUpdateHistory(store, source_list);
  for (vector<ScuttleMessage>::iterator it = hist.begin(); it != hist.end(); ++it) {
    cout << it->value << endl;
  }
}

// For this test, the payload should be an array whose first value is the key
void applyUpdate(const ScuttleMessage & m) {
  json_t * root;
  json_error_t err;
  root = json_loads(m.value.c_str(), 0, &err);
  if (!json_is_array(root)) return;
  string key = json_string_value(json_array_get(root,0));
  if (key == "__proto__") return;
  map<string,ScuttleMessage>::iterator it = store.find(key);
  if ( (it != store.end()) && (it->second.version > m.version) ) {
    //This is old data...
    return;
  }
  store[key] = m;
  json_decref(root);
  return;
}

void alloc_buffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
  memset(buf->base, 0, suggested_size); // don't know if this is necessary...
}

void on_close(uv_handle_t * handle) {
  cout << "remote endpoint closed connection" << endl;
}

// Read the data from the server and parse it.
void parse_read(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *) stream, on_close);
  } else if (nread > 0) {
    stringstream ss(buf->base);
    while(ss.peek() != EOF) {
      sc.getMessage(ss,applyUpdate);
    }
  }
  if (buf->base)
    free(buf->base);
}

void on_connection(uv_connect_t * req, int status) {
  if (status < 0) return;
  // setup the buffer containing the digest message on the stack...
  string out = sc.getDigest() + "\n";
  char outstr[out.size()+1];
  strncpy(outstr, out.c_str(), out.size()+1);
  outstr[out.size()] = '\0';
  uv_buf_t outbuf = uv_buf_init(outstr, out.length());
  
  //setup the read callback, and send out digest message...
  stream = req->handle;
  uv_write_t write_req;
  
  uv_read_start((uv_stream_t *)stream, alloc_buffer, parse_read);
  uv_write(&write_req, stream, &outbuf, 1, NULL);
}

void on_sync() {
  cout << "Received SYNC" << endl;
  //take the remote digest and calculate updates that need to be sent
  vector<ScuttleMessage> hist = sc.getUpdateHistory(store, remote_digest);
  for (vector<ScuttleMessage>::iterator it = hist.begin(); it != hist.end(); ++it) {
    string out = it->toJSON();
    char outstr[out.size()+1];
    strncpy(outstr, out.c_str(), out.size()+1);
    outstr[out.size()] = '\0';
    uv_buf_t outbuf = uv_buf_init(outstr, out.size());
    uv_write_t write_req;
    uv_write(&write_req, stream, &outbuf, 1, NULL);
    
  }
  char outstr[8] = "\"SYNC\"\n";
  outstr[7] = '\0';
  uv_buf_t outbuf = uv_buf_init(outstr, 7);
  uv_write_t write_req;
  uv_write(&write_req, stream, &outbuf, 1, NULL);
  cout << "finished processing handshake - sent SYNC" << endl;
  
  //cout << "Closing connection and printing out store contents:" << endl << endl;
  //uv_close((uv_handle_t *) stream, NULL);
  //outputStoreContents();
}

void on_handshake(map<string,double> digest) {
  remote_digest = digest;
}

int main() {  
  sc.setSyncCallback(on_sync);
  sc.setHandshakeCallback(on_handshake);
  string id = sc.createID();
  
  cout << "Some basic protocol only tests" << endl;
  string msg1 = "[[\"Value 1\", {\"id\":\"Value 1\", \"val\":1}], 1386248706714.002, \"FAKESOURCE\"]\n";
  sc.parseLine(msg1, applyUpdate);
  outputStoreContents();
  msg1 = "[[\"Value 1\", {\"id\":\"Value 1\", \"val\":2}], 1386248706714.003, \"FAKESOURCE\"]\n";
  sc.parseLine(msg1, applyUpdate);
  outputStoreContents();
  msg1 = "[[\"Value 1\", {\"id\":\"Value 1\", \"val\":3}], 1386248706715.002, \"FAKESOURCE2\"]\n";
  sc.parseLine(msg1, applyUpdate);
  outputStoreContents();
  msg1 = "[[\"Value 1\", {\"id\":\"Value 1\", \"val\":4}], 1386248706714.002, \"FAKESOURCE2\"]\n";
  sc.parseLine(msg1, applyUpdate);
  outputStoreContents();
  
  cout << "history?" << endl;
  map<string,double> source_list;
  source_list["FAKESOURCE"] = 1386248706714.002;
  outputHistoryContents(source_list);
  
  string out = sc.getDigest();
  
  cout << "DIGEST: " << out << endl;
  cout << "done" << endl;
  //return 0;
  
  uv_tcp_t socket;
  uv_tcp_init(uv_default_loop(), &socket);
  uv_connect_t connect;
  
  struct sockaddr_in dest_addr;
  uv_ip4_addr("127.0.0.1", 8124, &dest_addr);
  uv_tcp_connect(&connect, &socket, (sockaddr *)&dest_addr, on_connection);
  
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  
}
