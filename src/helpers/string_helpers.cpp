#include <cctype>
#include <string>
using namespace std;
/*
-------REASONING-------
-string string - -
string string - -
string string -
se forma el string
string(start, end) the superior lim is exclusive though
(string, string)- (the final space is not added to the final string)
*/

string trim(const string &str) {
  auto start = str.begin();
  auto end = str.end();

  while (start != end && isspace(*start)) {
    ++start;
  }

  while (end != start && isspace(*(end - 1))) {
    --end;
  }

  return string(start, end);
}
