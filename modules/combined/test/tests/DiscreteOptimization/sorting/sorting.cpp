#include <iostream>
#include <vector>
#include <algorithm>

// To test the sorting process (to verify the best_solutions pair sorting for both +ve and -ve
// objective values).

int
main()
{
  // A vector of pairs, where the first element is a negative objective function value
  std::vector<std::pair<double, int>> best_solutions_negative = {{-1.5, 1}, {-3.5, 2}, {-2.5, 3}};

  std::vector<std::pair<double, int>> best_solutions_positive = {{1.5, 1}, {3.5, 2}, {2.5, 3}};

  std::vector<std::pair<double, int>> best_solutions_repeated_negative = {
      {-1.5, 1}, {-2.5, 2}, {-2.5, 3}};

  std::vector<std::pair<double, int>> best_solutions_repeated_positive = {
      {2.5, 1}, {3.5, 2}, {2.5, 3}};

  // Sort in ascending order based on the first element of the pairs
  std::sort(best_solutions_negative.begin(), best_solutions_negative.end());
  std::sort(best_solutions_positive.begin(), best_solutions_positive.end());
  std::sort(best_solutions_repeated_negative.begin(), best_solutions_repeated_negative.end());
  std::sort(best_solutions_repeated_positive.begin(), best_solutions_repeated_positive.end());

  // Print the sorted values
  std::cout << "Sorted objective function values (Positive):" << std::endl;
  for (const auto & pair : best_solutions_positive)
  {
    std::cout << "Objective value: " << pair.first << ", Associated data: " << pair.second
              << std::endl;
  }

  // Print the sorted values
  std::cout << "Sorted objective function values (Negative):" << std::endl;
  for (const auto & pair : best_solutions_negative)
  {
    std::cout << "Objective value: " << pair.first << ", Associated data: " << pair.second
              << std::endl;
  }

  // Print the sorted values
  std::cout << "Sorted objective function values (Repeated Negative):" << std::endl;
  for (const auto & pair : best_solutions_repeated_negative)
  {
    std::cout << "Objective value: " << pair.first << ", Associated data: " << pair.second
              << std::endl;
  }

  // Print the sorted values
  std::cout << "Sorted objective function values (Repeated Positive):" << std::endl;
  for (const auto & pair : best_solutions_repeated_positive)
  {
    std::cout << "Objective value: " << pair.first << ", Associated data: " << pair.second
              << std::endl;
  }

  return 0;
}
