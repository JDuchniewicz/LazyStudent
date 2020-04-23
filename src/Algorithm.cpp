#include <Algorithm.hpp>
#include <iostream>


void Algorithm::setParameters(bool enableElitism, int elitismPercent, int crossoverPercent)
{
    m_elitismEnabled = enableElitism;
    m_elitismPercent = elitismPercent;
    m_crossoverPercent = crossoverPercent;
}

void Algorithm::run()
{
    int generation = 0;
    bool found = false;

   generateInitialPopulation();

    while (!found)
    {
        std::sort(m_population.begin(), m_population.end(), IndividualPred()); // sort with lowest fitness first

        if (m_population.at(0).fitness <= 0)
        {
            found = true;
            break;
        }
        // if we reached many iterations and do not make progress?

        generateNewGeneration();
        std::cout << "Generation: " << generation << std::endl;
        std::cout << "Subject choice: ";
        printIndividual(m_population.at(0).chromosome);
        std::cout << "Fitness choice: " << m_population.at(0).fitness << std::endl;

        ++generation;
    }
    std::cout << "Final Generation" << std::endl;
    std::cout << "Generation: " << generation << std::endl;
    std::cout << "Subject choice: ";
    printIndividual(m_population.at(0).chromosome);
    std::cout << "Fitness choice: " << m_population.at(0).fitness << std::endl;
    // found the optimal solution
    // print it out?
}

void Algorithm::printIndividual(const std::vector<std::deque<bool>>& chromosome) const
{
    for (size_t i = 0; i < chromosome.size(); ++i)
    {
        std::cout << "S: " << i << " | ";
        for (size_t j = 1; j < chromosome.at(i).size(); ++j)
        {
            bool v = chromosome.at(i).at(j);
            std::cout << v << " ";
        }
        std::cout << "\t";
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
    int requiredSubjectsPerSem = std::ceil(m_minECTS/m_meanECTSPerSubject);
    std::vector<std::deque<bool>> chromosome;
    std::deque<int> subjectsIDs;
    subjectsIDs.resize(m_subjects.size());
    bool ready = false;
    
    while (!ready)
    {
        chromosome.clear();
        ready = true;
        for (const auto& sub : m_subjects) // can I simplify it further?
            subjectsIDs.push_back(sub.ID);
        // this should be shrinking container, shuffled and allowing for pop()
        std::shuffle(subjectsIDs.begin(), subjectsIDs.end(), m_randomEng);

        // choose  subjects randomly for each semester, in number equal to requiredECTS/meanECTS
        for (int i = 0; i < m_semesters; ++i)
        {
            std::deque<bool> chosen(m_subjects.size() + 1); // +1 for indices from '1'
            for (int j = 0; j < requiredSubjectsPerSem; ++j)// may happen that because of ceil it will assert, as deque will be empty
            {
                int id = subjectsIDs.back();
                subjectsIDs.pop_back();
                chosen.at(id) = true;
            }
            chromosome.push_back(chosen); // should trigger copy ellision
        }
        // validate if all semesters account for a valid generation
        ready = checkPermutation(chromosome);
    }

    return chromosome;
}

bool Algorithm::checkPermutation(const std::vector<std::deque<bool>>& chromosome) const
{
    std::deque<bool> unlocked(m_subjects.size() + 1);
    for (const auto& sub : m_freeSubjects)
        unlocked.at(sub.ID) = true;

    for (const auto& sem : chromosome)
    {
        for (size_t i = 1; i < sem.size(); ++i)
        {
            if (sem.at(i))
            {
                // check if it has dependencies, check if they are unlocked already
                auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(), [&](const auto& dep){
                    return static_cast<size_t>(dep.second) == i;
                });
                if (it != m_dependencies.end())
                {
                    for (const auto& dep : (*it).first)
                    {
                        // this subject has unmet dependencies, discard it
                        if (!unlocked.at(dep))
                            return false;
                    }
                }
                // if there is already a subject with this ID
                if (unlocked.at(i))
                    return false;
                // if ok mark it as unlocked
                unlocked.at(i) = true;
            }
        }
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
    std::uniform_int_distribution<> distr(0, threshold); //tweak the distribution?
    for (int i = 0; i < remainderNumber; ++i)
    {
        int randOne = distr(m_randomEng);
        int randTwo = distr(m_randomEng);
        Individual offspring = generateOffspring(m_population.at(randOne), m_population.at(randTwo));
        newGeneration.push_back(offspring);
    }
    
    m_population = newGeneration;
}

// decide on mating techniques, how to cross genes? whole choices of subjects per one semester? split semesters?
// for now simple mating, swap semesters etc
// allow for more advanced mating?
Individual Algorithm::generateOffspring(const Individual& parentOne, const Individual& parentTwo) const
{
    std::uniform_int_distribution<> distr(0, 100 - 1); //tweak the distribution?
    std::vector<std::deque<bool>> chromosome;
    bool ready = false;

    while (!ready)
    {
        for (size_t i = 0; i < parentOne.chromosome.size(); ++i)
        {
            int p = distr(m_randomEng);
            if (p < 45)
                chromosome.push_back(parentOne.chromosome.at(i));
            else if (p < 90)
                chromosome.push_back(parentTwo.chromosome.at(i));
            else
                chromosome.push_back(mutate());
        }

        ready = checkPermutation(chromosome);
    }
    return { chromosome, calculateFitness(chromosome)};
}

std::deque<bool> Algorithm::mutate() const
{
    int requiredSubjectsPerSem = std::ceil(m_minECTS/m_meanECTSPerSubject);
    std::deque<int> subjectsIDs;
    subjectsIDs.resize(m_subjects.size());
    for (const auto& sub : m_subjects)
        subjectsIDs.push_back(sub.ID);

    std::shuffle(subjectsIDs.begin(), subjectsIDs.end(), m_randomEng);
    
    std::deque<bool> chosen(m_subjects.size() + 1);
    for (int j = 1; j < requiredSubjectsPerSem; ++j)// may happen that because of ceil it will assert, as deque will be empty
    {
        int id = subjectsIDs.back();
        subjectsIDs.pop_back();
        chosen.at(id) = true;
    }

    return chosen;
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
                semVal += m_IDtoSubject.at(i).studyDays;
        }
        total += semVal;
    }
    return total - ideal;
}