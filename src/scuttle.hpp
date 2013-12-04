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

#include "message.hpp"

using namespace std;

class ScuttleButt {
  public:
    ScuttleButt() {};
    
    double getTimeStamp();
    string createID();
    string getMessage(iostream &);
    string getDigest();
    void parseLine(const string & str, void(*callbackFunction)(const string &, const Message &) = NULL);
    bool filter(const Message &, map<string,double>);
    vector<Message> getUpdateHistory(map<string,Message> &, map<string,double>);
    map<string,Message> getStore();
  
  private:
    string client_id;
    map<string,double> sources;
};

