#include <string>

class ScuttleMessage {
  public:
    ScuttleMessage() {};
    std::string value;   //payload
    std::string id;      // source id
    double version; // timestamp... seconds since epoch, incl. fractional part
};
