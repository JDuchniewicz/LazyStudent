#pragma once

#include <vector>
#include <utility>
#include <deque>
#include <unordered_map>
#include <random>

struct Subject
{
    int ID = 0;
    int ECTS = 0;
    int studyDays = 0;
};

struct Individual
{
    Individual(std::vector<std::deque<bool>> chr, int fit) : chromosome(std::move(chr)), fitness(fit) {};

    std::vector<std::deque<bool>> chromosome;
    int fitness;
};

struct IndividualPred
{
    inline bool operator()(const Individual& lhs, const Individual& rhs) { return lhs.fitness > rhs.fitness; }
};

class Algorithm
{
public:
    Algorithm(int semesters, int minECTS, int studyDays, std::vector<Subject> subjects, std::vector<std::pair<int, int>> dependencies)
        :  m_semesters(semesters), m_minECTS(minECTS), m_studyDays(studyDays), m_subjects(std::move(subjects))
    {
        // prepare dependencies
        for (const auto& [required, unlocked] : dependencies)
        {
            auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(), [&](auto& deps){ //once GCC 10 is out, structured bindings in lambdas will be enabled
                return deps.second == unlocked;
            });
            // if such depender already exists, push back new dependency
            if (it != m_dependencies.end())
                (*it).first.push_back(required);
            else
                m_dependencies.push_back({ { required }, unlocked });
        }

// In this algorithm free subject may be useless
//TODO
        // populate free subjects list only with elements that are not present as dependers
        std::copy_if(m_subjects.begin(), m_subjects.end(), std::back_inserter(m_freeSubjects), [&](auto subject){ //clarify it with gcc 10
            return (std::find_if(m_dependencies.begin(), m_dependencies.end(), [&](const auto& depPair){
                return depPair.second == subject.ID;
            }) == m_dependencies.end());
        });

        // TODO: think if this is correct, duplicate ID
        int sum = 0;
        for (const auto& sub : m_subjects)
        {
            m_IDtoSubject.emplace(sub.ID, sub);
            sum += sub.ECTS;
        }
        m_meanECTSPerSubject = sum / m_subjects.size();

        m_randomEng =  std::default_random_engine{};
    };
    
    void run();
private:
    void generateInitialPopulation();
    void generateNewGeneration();
    std::vector<std::deque<bool>> generateChromosome() const;
    Individual generateOffspring(const Individual& parentOne, const Individual& parentTwo);
    int calculateFitness(const std::vector<std::deque<bool>>& chromosome) const;

    int m_semesters;
    int m_minECTS;
    int m_studyDays;
    std::vector<Subject> m_subjects; // think about removing free ones from here? std::difference?

    int m_meanECTSPerSubject;
    std::vector<Subject> m_freeSubjects; // subjects which do not have any dependees
    std::vector<std::pair<std::vector<int>, int>> m_dependencies; // second depends on all from first
    std::vector<Individual> m_population;
    std::unordered_map<int, Subject> m_IDtoSubject;
    std::default_random_engine m_randomEng;
};