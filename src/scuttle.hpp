#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <jansson.h>

#include "scuttlemessage.hpp"

using namespace std;

class ScuttleButt {
  public:
    ScuttleButt() {};
    
    double getTimeStamp();
    string createID();
    void getMessage(iostream &, void(*callbackFunction)(const ScuttleMessage &) = NULL);
    string getDigest();
    void parseLine(const string & str, void(*callbackFunction)(const ScuttleMessage &) = NULL);
    bool filter(const ScuttleMessage &, map<string,double>);
    vector<ScuttleMessage> getUpdateHistory(map<string,ScuttleMessage> &, map<string,double>);
  
    map<string,double> getSources();
  
    void setSyncCallback(void(*cb)());
    void setHandshakeCallback(void(*cb)(map<string,double>));
    
  private:
    string client_id;
    map<string,double> sources;
    void (*sync)();
    void (*handshake)(map<string,double>);
};

