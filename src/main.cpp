#include <Algorithm.hpp>

int main()
{
    int N, k, m, popSize; // arbitrary values for now
    N = 3; // nr of semesters
    k = 4; // min ECTS per sem
    m = 10; // max study days per sem
    popSize = 100;
    /*
    std::vector<Subject> subjects {
        {1, 4, 30}, 
    };
    std::vector<std::pair<int, int>> dependencies {

    };
    */
    std::vector<Subject> subjects {
        {1, 1, 2}, {2, 2, 4}, {3, 2, 3}, {4, 3, 4}, {5, 1, 4}, {6, 2, 5}, {7, 1, 1}, {8, 1, 1}, {9, 2, 2}, {10, 5, 5}, {11, 4, 10}, {12, 1, 2}, {13, 1, 4} };
    std::vector<std::pair<int, int>> dependencies {
        {1, 2}, {1, 5}, {5, 6}, {2, 6}};
    Algorithm algo(popSize, N, k, m, subjects, dependencies);
    algo.setParameters();
    algo.run(); 
    return 0;
}