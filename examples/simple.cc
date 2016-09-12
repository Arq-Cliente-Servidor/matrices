#include <iostream>
#include <thread>
#include <vector>

using namespace std;

void count(int val, int &result) {
  int c = 0;
  for (int i = 0; i < val; i++)
    c = c + 1;
  result = c;
}

int main() {
  int res;
  // concurrencia de estado compartido
  thread t(count, 50, ref(res));
  thread u(count, 50, ref(res));
  // punto de sincronización
  t.join();
  u.join();
  cout << "Result: " << res << endl;
  return 0;
}
