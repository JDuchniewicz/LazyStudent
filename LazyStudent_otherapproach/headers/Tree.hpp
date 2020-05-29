#pragma once

#include <vector>
#include <queue>
#include <utility>
#include <memory>
#include <algorithm>

namespace
{
    static unsigned int maxID = 0;
}

static inline unsigned int generateID() { return maxID++; }

struct Subject
{
    int ID = 0;
    int ECTS = 0;
    int studyDays = 0;
};

struct Node
{
    Node() {};
    Node(int sd, int id) :studyDays(sd), ID(id) {};

    int studyDays = 0;
    int ID = 0;
    int level = 0;
    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;
    std::vector<int> pastSubjects; // useful for checking for already unlocked paths when generating // should it be in parent only? no need to duplicate // but they may be needed for keeping the total track of courses avail so far
    std::vector<int> chosenSubjects; // holds ID's of chosen subjects
};

struct NodeSortPred 
{
    inline bool operator() (const Node* lhs, const Node* rhs) const
    {
        return (lhs->studyDays < rhs->studyDays);
    }
};

// optimization idea for A*, don't construct full tree, just these nodes that have a chance to be proper ones
class Tree
{
public:
    Tree(int levels, int minECTS, int studyDays, std::vector<Subject> subjects, std::vector<std::pair<int, int>> dependencies)
        : m_root(nullptr), m_levels(levels), m_minECTS(minECTS), m_studyDays(studyDays), m_subjects(std::move(subjects))
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

        // populate free subjects list only with elements that are not present as dependers
        std::copy_if(m_subjects.begin(), m_subjects.end(), std::back_inserter(m_freeSubjects), [&](auto subject){ //clarify it with gcc 10
            return (std::find_if(m_dependencies.begin(), m_dependencies.end(), [&](const auto& depPair){
                return depPair.second == subject.ID;
            }) == m_dependencies.end());
        });
    };
    
    void buildTree();

    void printTree() const; //util function

    void findCheapestPath() const;
    int runAStar(Node* end, std::vector<Node*>& path) const;
private:
    void addChildren(Node* parent, std::queue<Node*>& q);
    void generateCombination(const std::vector<Subject>& input, size_t idx, bool chosen[], std::queue<Node*>& q, Node* parent);
    bool canAddToQueue(const std::vector<Subject>& input, bool chosen[]) const;
    void addNode(const std::vector<Subject>& input, bool chosen[], std::queue<Node*>& q, Node* parent);
    int calculateHeuristicCost(Node* node);

    std::unique_ptr<Node> m_root;
    int m_levels;
    int m_minECTS;
    int m_studyDays;
    std::vector<Subject> m_subjects; // think about removing free ones from here? std::difference?
    std::vector<Subject> m_freeSubjects; // subjects which do not have any dependees
    std::vector<std::pair<std::vector<int>, int>> m_dependencies; // second depends on all from first
    mutable std::vector<Node*> m_lastNodes;
};