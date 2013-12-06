#include "scuttle.hpp"

bool sort_function (const ScuttleMessage & a, const ScuttleMessage & b) {
  return (a.version < b.version) || (a.id < b.id);
}

void ScuttleButt::setSyncCallback(void(*cb)()) {
  sync = cb;
}

void ScuttleButt::setHandshakeCallback(void(*cb)(map<string,double>)) {
  handshake = cb;
}

bool ScuttleButt::filter(const ScuttleMessage & update, map<string,double> source_list) {
  string src = update.id;
  double ts = update.version;
  return (source_list.find(src) == source_list.end()) || source_list.find(src)->second < ts;
}

map<string,double> ScuttleButt::getSources() {
  return sources;
}

vector<ScuttleMessage> ScuttleButt::getUpdateHistory(map<string,ScuttleMessage> & store, map<string,double> source_list) {
  vector<ScuttleMessage> m;
  for(map<string,ScuttleMessage>::iterator it = store.begin(); it != store.end(); ++it) {
    if ( filter(it->second, source_list) ) {
      m.push_back(it->second);
    }
  }
  sort(m.begin(), m.end(), sort_function);
  return m;
}

string ScuttleButt::createID() {
  srand(time(NULL));
  
  char buf[8];
  string retval;
  memset(buf, 0, sizeof(char)*8);
  snprintf(buf, 9, "%X", rand());
  retval += buf;
  memset(buf, 0, sizeof(char)*8);
  snprintf(buf, 9, "%X", rand());
  retval += buf;
  memset(buf, 0, sizeof(char)*8);
  snprintf(buf, 9, "%X", rand());
  retval += buf;
  
  client_id = retval;
  
  return retval;
}

void ScuttleButt::getMessage(iostream & stream, void(*callbackFunction)(const ScuttleMessage &)) {
  string line;
  getline(stream, line);
  parseLine(line, callbackFunction);
}

/** Also known as the "digest", it contains a list of all the
 *  sources this node has data from and the most recent timestamp
 *  of data it has received from each of them.
 */
string ScuttleButt::getDigest() {
  if (client_id.empty()) return "";
  
  json_t * root;
  double timestamp = getTimeStamp();
  root = json_pack("{sss{}}","id",client_id.c_str(),"clock");
  json_t * j_clock = json_object_get(root, "clock");
  for(map<string,double>::iterator it = sources.begin(); it != sources.end(); ++it) {
    json_object_set(j_clock, it->first.c_str(), json_real(it->second));
  }
  // We do this little dance because we have to free the memory...
  char * s = json_dumps(root,JSON_COMPACT);
  string retval = s;
  free(s);
  json_decref(root);
  return retval;
}

void ScuttleButt::parseLine(const string & str, void(*callbackFunction)(const ScuttleMessage &)) {
  cout << "parsing message: " << str << endl;
  if (str == "\"SYNC\"") {
    if (sync) sync();
    return;
  }
  
  json_t * root;
  json_error_t err;
  root = json_loads(str.c_str(), 0, &err);
  if (!root) {
    cerr << "problem parsing (" << str.length() << "): " << str << endl;
    return;
  }
  if (json_is_object(root)) {
    json_t * data = json_object_get(root, "id");
    if (data) {
      //cout << "Server Digest: " << str << endl;
      json_t * digest_data = json_object_get(root, "clock");
      if (clock && handshake) {
        map<string,double> digest;
        const char * key;
        json_t * value;
        json_object_foreach(digest_data, key, value) {
          //cout << key << " : " << json_real_value(value) << " -- from server digest" << endl;
          digest[key] = json_real_value(value);
        }
        handshake(digest);
      }
    }
  } else if (json_is_array(root)) {
    json_t * data = json_array_get(root,0);
    //if (!json_is_array(data)) return;
    //json_t * key  = json_array_get(data,0);
    //json_t * value = json_array_get(data,1);
    json_t * j_ts = json_array_get(root,1);
    json_t * j_id = json_array_get(root,2);
    double now = getTimeStamp();
    string id_value = "";
    double ts_value = 0.0;
    if (j_ts && j_id) {
      id_value = json_string_value(j_id);
      if (!json_is_real(j_ts)) {
        ts_value = json_integer_value(j_ts) + 0.0;
      } else {
        ts_value = json_real_value(j_ts);
      }
      map<string,double>::iterator it = sources.find(json_string_value(j_id));
      if (it != sources.end()) {
        if (it->second >= ts_value) {
          json_decref(root);
          return; // ignore old data
        }
      }
      sources[id_value] = ts_value;
    }

    if (callbackFunction) {
      ScuttleMessage m;
      m.id = id_value;
      m.version = ts_value;
      // We don't want to impose strict requirements on the payload...
      char * message_payload = json_dumps(data, JSON_COMPACT | JSON_ENCODE_ANY);
      m.value = message_payload;
      free(message_payload);
      json_decref(root);
      callbackFunction(m);
    }
  }
  json_decref(root);
}

double ScuttleButt::getTimeStamp() {
  timeval curTime;
  gettimeofday(&curTime, NULL);
  double dTime = curTime.tv_sec + (curTime.tv_usec / 1000.0);
  return dTime;
}

