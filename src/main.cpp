#include <Algorithm.hpp>
#include <sstream>
#include <limits>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Please specify input file name." << std::endl;
        exit(-1);
    }
    int N = 0, k = 0, m = 0, popSize = 100;
    std::vector<Subject> subjects;
    std::vector<std::pair<int, int>> dependencies;

    std::string name(argv[1]);
    std::ifstream file(name, std::ios::in);
    if (!file.is_open())
    {
        std::cerr << "Failed to open " << argv[1] << std::endl;
        exit(-1);
    }
    file >> N >> k >> m;
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // to advance to next line

    int i = 1;
    std::string line;

    subjects.push_back({ -1, -1 }); // for the 0'th element
    // read subjects
    while (std::getline(file, line))
    {
        if (line == "#")
            break;
        std::istringstream str(line);
        int days = 0;
        int ects = 0;
        str >> days;
        str >> ects;
        subjects.push_back({ ects, days });
        //std::cout << "Read " << i << " " << ects << " " << days << std::endl;
        ++i;
    }

    // read dependencies
    while (std::getline(file, line))
    {
        std::istringstream str(line);
        int dependent = 0;
        int what = 0;
        str >> dependent;
        str >> what;
        dependencies.push_back({dependent, what});
    //    std::cout << "Read " << dependent << " " << what << std::endl;
    }

    file.close();
    Algorithm algo(popSize, N, k, m, subjects, dependencies);
    algo.setParameters();
    algo.run(); 
    return 0;
}