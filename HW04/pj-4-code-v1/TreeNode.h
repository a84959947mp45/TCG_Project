#include <vector>
 
#include "action.h" 

class TreeNode{
    private:
        board::point m_move;
        int m_nChildren;
        int m_nBranchingFactor;
        float m_fValue;
        float m_fProbabilty;
        float m_fProbabiltyWithNoise;
        TreeNode* m_pFirstChild;

        float m_simCount ;
        float m_mean ;

        
    public:
        TreeNode() {}
        ~TreeNode() {}

        
        inline bool hasChildren() const { return (m_nChildren != 0); }

        inline void setFirstChild(TreeNode* pFirstChild) { m_pFirstChild = pFirstChild; }
        inline void setNumChild(int nChild) { m_nChildren = nChild; }
        inline void setProbability(float fProbability) { m_fProbabilty = fProbability; }
        inline void setValue(float fValue) { m_fValue = fValue; }
        inline void setSimCount(float simCount) { m_simCount = simCount; }
        inline void setMean(float mean) { m_mean = mean; }

        inline board::point getMove() const { return m_move; }
        inline TreeNode* getFirstChild() { return m_pFirstChild; }
        inline int getNumChild() const { return m_nChildren; }
        inline float getSimCount() const { return m_simCount; }
        inline float getMean() const { return m_mean; }


        void reset(const board::point & move)
        {
            m_move = move;
            m_nChildren = 0;
            m_nBranchingFactor = 0;
            m_fValue = 0.0f;
            m_fProbabilty = 0.0f;
            m_fProbabiltyWithNoise = 0.0f;
            m_simCount = 0;
            m_simCount = 0;
            m_mean = 0;

            m_pFirstChild = nullptr;
        }

        inline float getUCTValueQ(float fInitValueQ = -1.0f) const {
	        return (getSimCount() == 0 ? fInitValueQ : getMean());
        }

};