#include <Algorithm.hpp>
#include <chrono>

#define DEBUG 1

void Algorithm::setParameters(bool enableElitism, int elitismPercent, int crossoverPercent, int convergenceN)
{
    m_elitismEnabled = enableElitism;
    m_elitismPercent = elitismPercent;
    m_crossoverPercent = crossoverPercent;
    m_convergenceN = convergenceN;
}

void Algorithm::run()
{
    auto start = std::chrono::high_resolution_clock::now();
    int generation = 0;
    bool found = false;

   generateInitialPopulation();

    while (!found)
    {
        std::sort(m_population.begin(), m_population.end(), IndividualPred()); // sort with lowest fitness first

        if (checkConvergence(m_population.at(0).fitness))
        {
            found = true;
            break;
        }

        generateNewGeneration();
#if DEBUG
        std::cout << "Generation: " << generation << std::endl;
        std::cout << "Subject choice: ";
        printIndividual(m_population.at(0).chromosome);
        std::cout << "Fitness choice: " << m_population.at(0).fitness << std::endl;
#endif

        ++generation;
    }
#if DEBUG
    std::cout << "Final Generation" << std::endl;
    std::cout << "Generation: " << generation << std::endl;
    std::cout << "Subject choice: ";
    printIndividual(m_population.at(0).chromosome);
    std::cout << "Fitness choice: " << m_population.at(0).fitness << std::endl;
#endif
    printResult(m_population.at(0).chromosome);

    // print time
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Algorithm ran " << duration << " miliseconds." << std::endl;
}

void Algorithm::printIndividual(const std::vector<std::deque<bool>>& chromosome) const
{
    std::cout << std::endl;
    for (size_t i = 0; i < chromosome.size(); ++i)
    {
        std::cout << "S: " << i << " | ";
        for (size_t j = 1; j < chromosome.at(i).size(); ++j)
        {
            bool v = chromosome.at(i).at(j);
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

bool Algorithm::checkConvergence(int currentFitness)
{
    if (static_cast<int>(m_lastTenScores.size()) < m_convergenceN)
    {
        m_lastTenScores.push_back(currentFitness);
        return false;
    }
    std::rotate(m_lastTenScores.begin(), m_lastTenScores.begin() + 1, m_lastTenScores.end());
    m_lastTenScores.at(m_convergenceN - 1) = currentFitness;
    if (*m_lastTenScores.begin() == *(m_lastTenScores.end() - 1))
    {
        std::cout << "################################################" << std::endl;
        std::cout << "Converged - no change after " << m_convergenceN << " iterations." << std::endl;
        std::cout << "################################################" << std::endl;
        return true;
    }

    return false;
}

void Algorithm::printResult(const std::vector<std::deque<bool>>& chromosome) const
{
    std::cout << std::endl;
    for (size_t i = 0; i < chromosome.size(); ++i)
    {
        std::cout << "Semester: " << i << " | ";
        for (size_t j = 1; j < chromosome.at(i).size(); ++j)
        {
            bool v = chromosome.at(i).at(j);
            if (v)
                std::cout << j << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Algorithm::generateInitialPopulation()
{
    for (int i = 0; i < m_populationSize; ++i)
    {
        auto chromosome = generateChromosome();
        int fitness = calculateFitness(chromosome);
        Individual ind(chromosome, fitness);
        m_population.push_back(ind);
    }
}

// simpler version, generate combinations and prune these that do not satisfy conditions
std::vector<std::deque<bool>> Algorithm::generateChromosome() const
{
    std::vector<std::deque<bool>> chromosome;
    std::deque<int> subjectsIDs;
    bool ready = false;
    int tries = 0;
    
    while (!ready)
    {
        if (tries >= 10'000)
        {
            std::cout << "Cannot generate, not enough viable combinations for " << tries << " tries." << std::endl;
            exit(-1);
        }
        subjectsIDs.clear();
        subjectsIDs.resize(m_subjects.size() - 1);
        chromosome.clear();
        ready = true;

        std::iota(subjectsIDs.begin(), subjectsIDs.end(), 1);
        // this should be shrinking container, shuffled and allowing for pop()
        std::shuffle(subjectsIDs.begin(), subjectsIDs.end(), m_randomEng);

        for (int i = 0; i < m_semesters; ++i)
        {
            std::deque<bool> chosen(m_subjects.size());
            int semECTS = 0;
            while (semECTS < m_minECTS)
            {
                if (subjectsIDs.empty())// may happen that because of ceil it will assert, as deque will be empty
                    break;

                int id = subjectsIDs.back();
                subjectsIDs.pop_back();
                chosen.at(id) = true;

                // add this subjects constraints
                const auto& sub = m_subjects.at(id);
                semECTS += sub.ECTS;
            }
            chromosome.push_back(chosen); // should trigger copy ellision
        }
        // validate if all semesters account for a valid generation
        ready = checkPermutation(chromosome);
        ++tries;
    }

    return chromosome;
}

bool Algorithm::checkPermutation(const std::vector<std::deque<bool>>& chromosome) const
{
    std::deque<bool> chosen(m_subjects.size());
    for (const auto& sem : chromosome)
    {
        int semECTS = 0; // check the ECTS boundary
        int semStudyHours = 0;
        for (size_t i = 1; i < sem.size(); ++i)
        {
            if (sem.at(i))
            {
                // check if it has dependencies, check if they are unlocked or already chosen
                auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(), [&](const auto& dep){
                    return static_cast<size_t>(dep.second) == i;
                });
                if (it != m_dependencies.end())
                {
                    for (const auto& dep : (*it).first)
                    {
                        // this subject has unmet dependencies, discard it
                        if (!chosen.at(dep)) // if not already chosen in previous iterations, dependency not met
                            return false;
                    }
                }
                // if there is already a subject with this ID
                if (chosen.at(i))
                    return false;

                chosen.at(i) = true;
                // add this subjects constraints
                const auto& sub = m_subjects.at(i);
                semECTS += sub.ECTS;
                semStudyHours += sub.studyDays;
            }
        }
        // check if ECTS requirements met for this semester
        if (semECTS < m_minECTS || semStudyHours > m_studyDays)
            return false;
    }
    return true;
}

void Algorithm::generateNewGeneration()
{
    std::vector<Individual> newGeneration;
    int threshold = 0;
    int elitismNumber = ((m_elitismPercent * m_populationSize)/ 100);
    int remainderNumber = m_populationSize - elitismNumber;

    if (m_elitismEnabled)
    {
        for (int i = 0; i < elitismNumber; ++i)
            newGeneration.push_back(m_population.at(i));
    }

    threshold = ((m_crossoverPercent * m_populationSize)/ 100);
    std::uniform_int_distribution<> distr(0, threshold - 1); //tweak the distribution?
    for (int i = 0; i < remainderNumber; ++i)
    {
        int randOne = distr(m_randomEng);
        int randTwo = distr(m_randomEng);
        Individual offspring = generateOffspring(m_population.at(randOne), m_population.at(randTwo));
        newGeneration.push_back(offspring);
    }
    
    m_population = newGeneration;
}

Individual Algorithm::generateOffspring(const Individual& parentOne, const Individual& parentTwo) const
{
    std::uniform_int_distribution<> distr(0, 100 - 1); //tweak the distribution?
    std::vector<std::deque<bool>> chromosome;
    bool ready = false;
    int iterations = 0;

    while (!ready)
    {
        chromosome.clear();
        if (iterations > 10'000)
        {
            std::cout << "Could not generate offspring in: " << iterations << " iterations.";
            exit(-1);
        }
        for (size_t i = 0; i < parentOne.chromosome.size(); ++i)
        {
            int p = distr(m_randomEng);
            if (p < 90)
                chromosome.push_back(cross(parentOne.chromosome.at(i), parentTwo.chromosome.at(i)));
            else if (p < 95)
                chromosome.push_back(mutate(parentOne.chromosome.at(i)));
            else
                chromosome.push_back(mutate(parentTwo.chromosome.at(i)));
        }

        //ready = true;
       // std::cout << "Iteration: " << iterations << std::endl;
       // printIndividual(chromosome);
        ready = checkPermutation(chromosome);
        ++iterations;
    }
    return { chromosome, calculateFitness(chromosome)};
}

// flip just one subject choice in mutation, less invasive
std::deque<bool> Algorithm::mutate(const std::deque<bool>& gene) const
{
    std::uniform_int_distribution<> distr(0, gene.size() - 1); //tweak the distribution?
    std::deque<bool> mutated = gene;
    int p = distr(m_randomEng);
    mutated.at(p) = !gene.at(p);
    return mutated;
}

std::deque<bool> Algorithm::cross(const std::deque<bool>& geneOne, const std::deque<bool>& geneTwo) const
{
    std::uniform_int_distribution<> distr(0, geneOne.size() - 1); //tweak the distribution?
    std::deque<bool> crossed;
    int p = distr(m_randomEng);
    bool chosenOne = false;
    if (p > static_cast<int>((geneOne.size() / 2)))
    {
        crossed = geneOne;
        chosenOne = true;
    }
    else
        crossed = geneTwo;

    p = distr(m_randomEng) / 4; // check how many values have to be exchanged?
    int randIdx = 0;
    for (int i = 0; i < p; ++i)
    {
        randIdx = distr(m_randomEng);
        if (chosenOne)
            crossed.at(randIdx) = geneTwo.at(randIdx);
        else
            crossed.at(randIdx) = geneOne.at(randIdx);
    }

    return crossed;
}

int Algorithm::calculateFitness(const std::vector<std::deque<bool>>& chromosome) const
{
    // fitness determines how much does the algorithm differ from the ideal solution -> minimal days of studying 
    int ideal = m_semesters * m_studyDays; // precompute?
    int total = 0;
    for (const auto& semester : chromosome)
    {
        int semVal = 0;
        for (size_t i = 1; i < semester.size(); ++i)
        {
            if (semester.at(i))
                semVal += m_subjects.at(i).studyDays;
        }
        total += semVal;
    }
    return total - ideal;
}