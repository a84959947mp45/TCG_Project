#include "MCTS.h"
#include <stdlib.h>
#include <iomanip>
#include <time.h>

action::place MCTS::run(board& board , board::piece_type who, bool bWithPlay/* = true*/)
{
	board.setTurnColor(who);
	newTree(board);
	backupGame();

	clock_t startTime = clock();
	clock_t now = clock();;
	//while (!isSimulationEnd()) {
	while ((((double) (now - startTime)) / CLOCKS_PER_SEC) < 1) {
		selection();
		evaluation();
		expansion();
		update();
		rollbackGame();

		//++m_simulation;
		now = clock();
	}

	TreeNode* pSelected = decideMCTSAction();
	//cout<<pSelected->getMove()<<" "<<pSelected->getMean()<<endl;
	return action::place(pSelected->getMove().x,pSelected->getMove().y,pSelected->getMove().color);
}

void MCTS::selection()
{
	TreeNode* pNode = getRootNode();
	m_vSelectNodePath.clear();
	m_vSelectNodePath.push_back(pNode);
	while (pNode->hasChildren()) {
		pNode = selectChild(pNode);
		action::place(pNode->getMove(), pNode->getMove().color).apply(m_game);
		m_vSelectNodePath.push_back(pNode);
	}
}

void MCTS::evaluation()
{
    srand(time(NULL));
//	cout<<"---"<<endl;
  //  cout<<m_game<<endl;

	int num = 0;
	board::piece_type cTurn = m_game.getTurnColor();
	while (!m_game.isTerminal()) {
		vector<int> legalMoves;
		for (int p = 0; p < getMaxNumLegalAction(); ++p) {
			if (m_game.isLegalMove(board::point(p,m_game.getTurnColor()))) {
				legalMoves.push_back(p);
			}
		}
	
		int randMove = rand() % legalMoves.size();
		action::place(legalMoves[randMove],m_game.getTurnColor()).apply(m_game);
		++num;
        //cout<<"turn:"<<m_game.getTurnColor()<<endl;
	}
	//cout<<"who win:"<<m_game.eval()<<endl;
	m_fValue = (m_game.eval() == cTurn) ? 0 : 1;

	for (int i = 0; i < num; ++i) {
		m_game.undo();
	}
	m_vProbability.clear();
	for (int pos = 0; pos < getMaxNumLegalAction(); ++pos) {
		if (m_game.isLegalMove(board::point(pos,m_game.getTurnColor()))) {
			m_vProbability.push_back({board::point(pos, m_game.getTurnColor()), -1.});
		}
	}
	return;
}

void MCTS::expansion()
{
	if (m_game.isTerminal()) { return; }

	const vector<pair<board::point, float>>& vProbability = getProbability();
	TreeNode* pParent = m_vSelectNodePath.back();
	TreeNode* pChild = allocateNewNodes(vProbability.size());

	// assign info to parent node
	pParent->setFirstChild(pChild);
	pParent->setNumChild(vProbability.size());

	// assign policy to child nodes
	for (auto p : vProbability) {
		pChild->reset(p.first);
		pChild->setProbability(p.second);
		++pChild;
	} 
}

void MCTS::update()
{
	float fValue = getValue();
	m_vSelectNodePath.back()->setValue(fValue);
	// update value
	for (int i = static_cast<int>(m_vSelectNodePath.size()) - 1; i >= 0; --i) {
		TreeNode* pNode = m_vSelectNodePath[i];
		updateByWinLose(pNode, fValue);
		fValue = 1 - fValue; 
	}
}

void MCTS::updateByWinLose(TreeNode* pNode, float fValue)
{
    float newMean = (pNode->getMean()* pNode->getSimCount() + fValue) / (pNode->getSimCount()+1);
	pNode->setMean(newMean);
	pNode->setSimCount(pNode->getSimCount()+1);
}

TreeNode* MCTS::selectChild(TreeNode* pNode)
{
	TreeNode* pBest = nullptr;
	TreeNode* pChild = pNode->getFirstChild();

	float fBestScore = -10000;
	float fInitQValue = calculateInitQValue(pNode);
	//float fInitQValue = 0;
	int nParentSimulation = pNode->getSimCount();
	for (int i = 0; i < pNode->getNumChild(); ++i, ++pChild) {
		float fMoveScore = calculateMoveScore(pChild, nParentSimulation, fInitQValue);
		if (fMoveScore <= fBestScore) { continue; }

		fBestScore = fMoveScore;
		pBest = pChild;
	}

	return pBest;
}

float MCTS::calculateInitQValue(TreeNode* pNode)
{
	float fSumOfChildWins = 0.0f;
	float fSumOfChildSims = 0.0f;
	TreeNode* pChild = pNode->getFirstChild();
	for (int i = 0; i < pNode->getNumChild(); ++i, ++pChild) {
		if (!pChild->hasChildren()) { continue; }

		// we only consider the nodes which have been simulated
		fSumOfChildWins += pChild->getUCTValueQ(-1);
		fSumOfChildSims += 1;
	}

	// add additional one loss for other uninitialized node
	return (fSumOfChildWins - 1) / (fSumOfChildSims + 1);
}


float MCTS::calculateMoveScore(TreeNode* pNode, int nParentSim, float fInitQValue)
{
	// Q(s,a)
	float fValueQ = pNode->getUCTValueQ(fInitQValue);

	// U(s,a) = c_uct * sqrt(ln( N(s,b))) / N(s,a)
	float fValueU = 1.5 * sqrt(log(nParentSim) / (1. + pNode->getSimCount()));
	
    //cout<<"Q:"<<fValueQ<<endl;
	//cout<<"U:"<<fValueU<<endl;
	// action value = Q(s,a) + U(s,a) 
	return fValueQ + fValueU ;
}

TreeNode* MCTS::decideMCTSAction()
{
	TreeNode* pRoot = getRootNode();
	TreeNode* pSelected = nullptr;
	TreeNode* pChild = pRoot->getFirstChild();

	for (int i = 0; i < pRoot->getNumChild(); ++i, ++pChild) {
		float fCount = pChild->getSimCount();
		if (fCount == 0) { continue; }

		if (pSelected == nullptr || fCount > pSelected->getSimCount()) { pSelected = pChild; }
	}

	return pSelected;
}