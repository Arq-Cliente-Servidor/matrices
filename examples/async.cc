#include <future>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

using namespace std;

int count(int val) {
  cout << "count: " << endl;
  int c = 0;
  for (int i = 0; i < c; i++) {
    c += i + 1;
  }
  return c;
}

int main() {
  future<int> t = async(launch::async, count, 50);
  future<int> s = async(launch::async, count, 150);
  cout << t.get() + s.get() << endl;
  return 0;
}
