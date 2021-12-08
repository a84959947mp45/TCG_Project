#pragma once

#include "TreeNode.h"

using namespace std;

class MCTS{

    public:
        MCTS()
        {
            MCTS_SIMULATION_COUNT = 3200;
            long long int nodeSize = 1 + static_cast<long long int>(MCTS_SIMULATION_COUNT) * getMaxNumLegalAction();
            m_nodes = new TreeNode[nodeSize];
        }

        ~MCTS() { delete[] m_nodes; }

        action::place run(board& b, board::piece_type c, bool bWithPlay = true);

        // MCTS four phases
        void selection();
        void evaluation();
        void expansion();
        void update();

        inline bool isSimulationEnd() { return m_simulation >= MCTS_SIMULATION_COUNT; }
        inline int getSimulation() const { return m_simulation; }
        inline TreeNode* getRootNode() { return &m_nodes[0]; }
        int getMaxNumLegalAction() { return board::size_x * board::size_y; }
        inline const vector<pair<board::point, float>>& getProbability() { return m_vProbability; }
        inline const float& getValue() { return m_fValue; }

        TreeNode* decideMCTSAction();
        TreeNode* selectChild(TreeNode* pNode);
        float calculateInitQValue(TreeNode* pNode);
        float calculateMoveScore(TreeNode* pNode, int nParentSim, float fInitQValue);
        void updateByWinLose(TreeNode* pNode, float fValue);
        inline void backupGame() { m_backupMove = m_game.getMoves().size(); }
        inline void rollbackGame() {
            while (m_game.getMoves().size() > m_backupMove) { m_game.undo(); }
        }
        void newTree(board& board)
        {
            m_game = board;
            m_simulation = 0;
            MCTS_SIMULATION_COUNT = 500;
            getRootNode()->reset(board::point( -1, m_game.AgainstColor(m_game.getTurnColor())));
            
            m_nodeUsedIndex = 1;
            m_backupMove = -1;
            m_vSelectNodePath.clear();
        }

        inline TreeNode* allocateNewNodes(int size) {

            TreeNode* pNode = &m_nodes[m_nodeUsedIndex];
            m_nodeUsedIndex += size;
            return pNode;
        }

         
    private:
        int m_simulation;
        int MCTS_SIMULATION_COUNT;
	    long long m_nodeUsedIndex;
        unsigned int m_backupMove;
        TreeNode* m_nodes;
	    vector<TreeNode*> m_vSelectNodePath;
        board m_game;
        float m_fValue;
        vector<pair<board::point, float>> m_vProbability;
};