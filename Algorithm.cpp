#include <Algorithm.hpp>
#include <algorithm>


void Algorithm::run()
{
    int generation = 0;
    bool found = false;

   generatePopulation();

    while (!found)
    {
        std::sort(m_population.begin(), m_population.end(), IndividualPred); // sort with lowest fitness first

        if (m_population.at(0).fitness <= 0)
        {
            found = true;
            break;
        }
        // if we reached many iterations and do not make progress?

        generateNewGeneration();

        ++generation;
    }
}

void Algorithm::generatePopulation()
{
    for (int i = 0; i < m_population; ++i)
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
    subjectsIDs.reserve(m_subjects.size());
    bool ready = false;
    
    while (!ready)
    {
        ready = true;
        for (const auto& sub : m_subjects) // can I simplify it further?
            subjectsIDs.push_back(sub.ID);
        // this should be shrinking container, shuffled and allowing for pop()
        std::shuffle(subjectsIDs.begin(), subjectsIDs.end(), m_randomEng);

        // choose  subjects randomly for each semester, in number equal to requiredECTS/meanECTS
        for (int i = 0; i < m_semesters; ++i)
        {
            std::deque<bool> chosen(m_subjects.size());
            for (int j = 0; j < requiredSubjectsPerSem; ++j)// may happen that because of ceil it will assert, as deque will be empty
            {
                int id = subjectsIDs.back();
                subjectsIDs.pop_back();
                chosen.at(id) = true;
            }
            chromosome.push_back(chosen); // should trigger copy ellision
        }

        // validate if all semesters account for a valid generation
        std::deque<bool> unlocked(m_subjects.size());
        for (const auto& sub : m_freeSubjects)
            unlocked.at(sub.ID) = true;

        for (const auto& sem : chromosome)
        {
            for (const auto& sub : sem)
            {
                // check if all chosen subjects are already unlocked
                if (sub)
                {
                    // similar algo to Tree

                    // if failure
                    ready = false;
                    break;
                }
            }
        }
    }

    return chromosome;
}

void Algorithm::generateNewGeneration()
{

    // decide how to percform mutations and elitism etc
}

Individual Algorithm::generateOffspring(const Individual& parentOne, const Individual& parentTwo)
{
    // decide on mating techniques, how to cross genes? whole choices of subjects per one semester? split semesters?
    // for now simple mating, swap semesters etc

}

int Algorithm::calculateFitness(const std::vector<std::deque<bool>>& chromosome) const
{
    // fitness determines how much does the algorithm differ from the ideal solution -> minimal days of studying 
    int ideal = m_semesters * m_studyDays; // precompute?
    int total = 0;
    for (const auto& semester : chromosome)
    {
        int semVal = 0;
        for (int i = 0; i < semester.size(); ++i)
        {
            if (semester.at(i))
                semVal += m_IDtoSubject.at(i).studyDays;
        }
        total += semVal;
    }
    return semVal - ideal;
}