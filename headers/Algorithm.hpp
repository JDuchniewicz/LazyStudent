#pragma once

#include <vector>
#include <utility>
#include <deque>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <iostream>


struct Subject
{
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
    inline bool operator()(const Individual& lhs, const Individual& rhs) { return lhs.fitness < rhs.fitness; }
};

class Algorithm
{
public:
    Algorithm(int populationSize, int semesters, int minECTS, int studyDays, std::vector<Subject> subjects, std::vector<std::pair<int, int>> dependencies)
        :  m_populationSize(populationSize), m_semesters(semesters), m_minECTS(minECTS), m_studyDays(studyDays), m_subjects(std::move(subjects))
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

        std::random_device rd;
        m_randomEng = std::mt19937{rd()};
    };
    
    void setParameters(bool enableElitism = true, int elitismPercent = 10, int crossoverPercent = 50, int convergenceN = 20);
    void run();
private:
    void generateInitialPopulation();
    void generateNewGeneration();

    std::vector<std::deque<bool>> generateChromosome() const;
    int calculateFitness(const std::vector<std::deque<bool>>& chromosome) const;
    bool checkPermutation(const std::vector<std::deque<bool>>& chromosome) const;

    Individual generateOffspring(const Individual& parentOne, const Individual& parentTwo) const;
    std::deque<bool> mutate(const std::deque<bool>& gene) const;
    std::deque<bool> cross(const std::deque<bool>& geneOne, const std::deque<bool>& geneTwo) const;

    void printIndividual(const std::vector<std::deque<bool>>& chromosome) const;
    void printResult(const std::vector<std::deque<bool>>& chromosome) const;
    bool checkConvergence(int currentFitness);

    int m_populationSize;
    int m_semesters;
    int m_minECTS;
    int m_studyDays;
    std::vector<Subject> m_subjects;

    std::vector<std::pair<std::vector<int>, int>> m_dependencies; // second depends on all from first
    std::vector<Individual> m_population;
    mutable std::mt19937 m_randomEng;

    bool m_elitismEnabled;
    int m_elitismPercent;
    int m_crossoverPercent;

    std::vector<int> m_lastTenScores;
    int m_convergenceN;
};