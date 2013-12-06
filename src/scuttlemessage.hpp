#include <string>
#include <sstream>

class ScuttleMessage {
  public:
    ScuttleMessage() {};
    std::string value;   //payload
    std::string id;      // source id
    double version; // timestamp... seconds since epoch, incl. fractional part
    std::string toJSON() {
      std::stringstream ss;
      ss.setf( std::ios::fixed, std::ios::floatfield );
      ss.precision(3);
      ss << "[" << value << "," << version << ",\"" << id << "\"]\n";
      return ss.str();
    };
};
