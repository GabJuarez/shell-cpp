#include <iostream>
#include <string>
#include <vector>
using namespace std;

void echo(vector<string> args) {
  for (int i = 0; i < args.size(); i++) {
    cout << args[i] << " ";
  }
  cout << endl;
}
