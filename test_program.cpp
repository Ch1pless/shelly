#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  cout << "You've activated a test script\n";

  for (int i = 0; i < argc; ++i) {
    cout << i << ": " << argv[i] << "\n";
  }  
  cout << "The test script is finishing" << "\n";
  return 0;
}