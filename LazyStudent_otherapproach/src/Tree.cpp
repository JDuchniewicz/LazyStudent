#include <Tree.hpp>

#include <iostream>
#include <stack>

void Tree::buildTree()
{
    Node* current = nullptr;
    m_root = std::make_unique<Node>(-1, generateID()); // root node
    std::queue<Node*> q;
    q.push(m_root.get());
    while (!q.empty())
    {
        current = q.front();
        q.pop();
        addChildren(current, q); // add children for each node in each row
    }
}

void Tree::addChildren(Node* parent, std::queue<Node*>& q)
{
    if (parent->level == m_levels)
        return;

    std::vector<Subject> availSubjects;
    // find all free not yet used subjects
    for (const auto& [id, ects, days] : m_freeSubjects)
    {
        auto it = std::find_if(parent->pastSubjects.begin(), parent->pastSubjects.end(), [&](auto pastID){ return id == pastID; }); // past subjects should include chosen?
        auto it2 = std::find_if(parent->chosenSubjects.begin(), parent->chosenSubjects.end(), [&](auto chosenID){ return id == chosenID; });
        // if such subject not already used up the tree
        if (it == parent->pastSubjects.end() && it2 == parent->chosenSubjects.end())
            availSubjects.push_back({id, ects, days});
    }
    // suboptimal?
    for (const auto& [dependsOn, what] : m_dependencies)
    {
        bool unlocked = true;

        // if 'what' was already used, skip the whole dependency
        auto whatIt = std::find_if(parent->pastSubjects.begin(), parent->pastSubjects.end(), [&](auto pastID){ return what == pastID; });
        auto whatIt2 = std::find_if(parent->chosenSubjects.begin(), parent->chosenSubjects.end(), [&](auto chosenID){ return what == chosenID; });
        if (whatIt != parent->pastSubjects.end() || whatIt2 != parent->chosenSubjects.end())
            continue;

        for (auto id : dependsOn)
        {
            auto it = std::find_if(parent->pastSubjects.begin(), parent->pastSubjects.end(), [&](auto pastID){ return id == pastID; });
            auto it2 = std::find_if(parent->chosenSubjects.begin(), parent->chosenSubjects.end(), [&](auto chosenID){ return id == chosenID; });
            // check which of the prerequisites are satisfied and if all have been satisfied so far, then add the given subject to the available ones
            if (it == parent->pastSubjects.end() && it2 == parent->chosenSubjects.end())
            {
                unlocked = false;
                break;
            }
        }
        // if found then get 'what' subject and add it as available
        if (unlocked)
        {
            auto it = std::find_if(m_subjects.begin(), m_subjects.end(), [&](const auto& sub){ return sub.ID == what; });
            availSubjects.push_back(*it);
        }
    }
    if (availSubjects.size() == 0)
        return;
    // generate subsets of them fitting in k ECTS per semester
    // use inclusion-exclusion principle to generate all subsets then check if they satisfy a given condition
    bool* chosen = new bool[availSubjects.size()];
    size_t idx = 0;
    generateCombination(availSubjects, idx, chosen, q, parent);
    delete[] chosen;
}

void Tree::generateCombination(const std::vector<Subject>& input, size_t idx, bool chosen[], std::queue<Node*>& q, Node* parent)
{
    // for now generate all combinations that contain at least one node?
    // OPTIMIZATION -> delete those that are shit at this step already?
    if (idx > input.size() - 1)
    {
        if (canAddToQueue(input, chosen))
            addNode(input, chosen, q, parent);

        return;
    }
    // choose this element 
    chosen[idx] = true;
    generateCombination(input, idx + 1, chosen, q, parent);
    // not choose this element
    chosen[idx] = false;
    generateCombination(input, idx + 1, chosen, q, parent);
}

bool Tree::canAddToQueue(const std::vector<Subject>& input, bool chosen[]) const
{
    int currentDays = 0;
    int currentECTS = 0;
    for (size_t i = 0; i < input.size(); ++i)
    {
        if (chosen[i])
        {
            currentDays += input.at(i).studyDays;
            currentECTS += input.at(i).ECTS;
        }
    }
    if (currentECTS >= m_minECTS && currentDays <= m_studyDays)
        return true;

    return false;
}

void Tree::addNode(const std::vector<Subject>& input, bool chosen[], std::queue<Node*>& q, Node* parent)
{
    auto newNode = std::make_unique<Node>(); // addNewNode call, reparent it
    Node* raw = newNode.get();
    newNode->ID = generateID();
    newNode->level = parent->level + 1;
    newNode->parent = parent;
    newNode->pastSubjects = parent->pastSubjects; // add both past and current subjects of parent as past of this node
    newNode->pastSubjects.insert(newNode->pastSubjects.end(), parent->chosenSubjects.begin(), parent->chosenSubjects.end());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (chosen[i])
        {
            newNode->studyDays += input.at(i).studyDays;
            newNode->chosenSubjects.push_back(input.at(i).ID);
        }
    }
    if (newNode->level == m_levels)
        m_lastNodes.push_back(raw);

    parent->children.push_back(std::move(newNode));
    q.push(raw); // now it won't refer to trash
}

void Tree::printTree() const
{
    Node* current = nullptr;
    std::queue<Node*> q;
    q.push(m_root.get());
    while (!q.empty())
    {
        current = q.front();
        q.pop();
        for (const auto& n : current->children)
            q.push(n.get());
        
        std::cout << "Node level: " << current->level << " | ID: " << current->ID << " | days required: " << current->studyDays << " | past subjects: ";
        for (const auto& e : current->pastSubjects)
            std::cout << e << " ";
        std::cout << std::endl << " | chosen subjects: ";
        for (const auto& e : current->chosenSubjects)
            std::cout << e << " ";
        std::cout << std::endl;
    }
}

// TODO: add some measurements of time
void Tree::findCheapestPath() const
{
    if (m_lastNodes.empty())
        return;

    // find all nodes at level N
    std::sort(m_lastNodes.begin(), m_lastNodes.end(), NodeSortPred()); 
    Node* current = m_lastNodes.at(0);
    std::stack<Node*> stack;
    while (current != nullptr)
    {
        stack.push(current);
        current = current->parent;
    }
    std::cout << "Found semi-brute-force path" << std::endl;
    while (!stack.empty())
    {
        Node* curr = stack.top();
        stack.pop();
        std::cout << "Subjects for level: " << curr->level;
        for (const auto& sub : curr->chosenSubjects)
        {
            std::cout << " " << sub;
        }
        std::cout << std::endl;
    }
}
