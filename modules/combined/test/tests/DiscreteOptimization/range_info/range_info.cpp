#include <iostream>
#include <limits>
#include <cstdint> // For int16_t

// To test the computer ranges (to verify the max available for number of iterations and number of
// runs).

int
main()
{
  std::cout << "Size of unsigned int: " << sizeof(unsigned int) * 8 << " bits" << std::endl;
  std::cout << "Max value of unsigned int: " << std::numeric_limits<unsigned int>::max()
            << std::endl;
  std::cout << "Min value of unsigned int: " << std::numeric_limits<unsigned int>::min()
            << std::endl;

  // For unsigned long int
  std::cout << "Size of unsigned long int: " << sizeof(unsigned long int) * 8 << " bits"
            << std::endl;
  std::cout << "Max value of unsigned long int: " << std::numeric_limits<unsigned long int>::max()
            << std::endl;
  std::cout << "Min value of unsigned long int: " << std::numeric_limits<unsigned long int>::min()
            << std::endl;

  // For int
  std::cout << "\nSize of int: " << sizeof(int) * 8 << " bits" << std::endl;
  std::cout << "Max value of int: " << std::numeric_limits<int>::max() << std::endl;
  std::cout << "Min value of int: " << std::numeric_limits<int>::min() << std::endl;

  // For size_t
  std::cout << "\nSize of size_t: " << sizeof(size_t) * 8 << " bits" << std::endl;
  std::cout << "Max value of size_t: " << std::numeric_limits<size_t>::max() << std::endl;
  std::cout << "Min value of size_t: " << std::numeric_limits<size_t>::min() << std::endl;

  // For int16_t
  std::cout << "\nSize of int16_t: " << sizeof(int16_t) * 8 << " bits" << std::endl;
  std::cout << "Max value of int16_t: " << std::numeric_limits<int16_t>::max() << std::endl;
  std::cout << "Min value of int16_t: " << std::numeric_limits<int16_t>::min() << std::endl;

  // For uint16_t
  std::cout << "\nSize of uint16_t: " << sizeof(uint16_t) * 8 << " bits" << std::endl;
  std::cout << "Max value of uint16_t: " << std::numeric_limits<uint16_t>::max() << std::endl;
  std::cout << "Min value of uint16_t: " << std::numeric_limits<uint16_t>::min() << std::endl;

  return 0;
}
