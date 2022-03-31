// using atomic as a lock
#include <iostream>       // std::cout
#include <atomic>         // std::atomic
#include <thread>         // std::thread
#include <vector>         // std::vector
#include <sstream>        // std::stringstream

std::atomic<bool> lock_stream[10];
std::stringstream stream;

void append_number(int x) {
  while (lock_stream[x].load()) {}
  stream << "thread #" << x << '\n';
  stream.flush();
  if (x != 9) lock_stream[x + 1].store(false);
}

int main ()
{
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) lock_stream[i].store(true);
  for (int i=0; i<10; ++i) threads.push_back(std::thread(append_number,i));
  lock_stream[0].store(false);
  for (auto& th : threads) th.join();

  std::cout << stream.str();
  return 0;
}
