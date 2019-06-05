// linear_congruential_engine::operator()
#include <iostream>
#include <chrono>
#include <random>
#include <cstdlib>
#include <time.h>

int main ()
{
  // obtain a seed from the system clock:
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::minstd_rand0 generator (seed);  // minstd_rand0 is a standard linear_congruential_engine
  std::cout << "Random value: " << generator() << std::endl;

  srand(time(NULL));
  std::cout << rand() % 10 << std::endl;

  return 0;
}