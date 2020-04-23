#include <Algorithm.hpp>

int main()
{
    int N, k, m, popSize; // arbitrary values for now
    N = 3;
    k = 10;
    m = 50;
    popSize = 100;
    std::vector<Subject> subjects {
        {1, 10, 20}, {2, 10, 20}, {3, 8, 20}, {4, 5, 15}, {5, 10, 30}, {6, 10, 30}, /*{7, 15, 45}, {8, 3, 5} */ };
    std::vector<std::pair<int, int>> dependencies {
        {1, 2}, {1, 5}, {5, 6}, {2, 6}};
    Algorithm algo(popSize, N, k, m, subjects, dependencies);
    algo.run(); 
    return 0;
}