#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentNodeRecord;

		NodeRecord startRecord;
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(startRecord);

		while (!openList.empty())
		{
			auto it{ std::min_element(openList.begin(), openList.end()) };
			currentNodeRecord = (*it);

			if (currentNodeRecord.pNode == pGoalNode)
			{
				break;
			}

			for (auto con : m_pGraph->GetNodeConnections(currentNodeRecord.pNode->GetIndex()))
			{
				T_NodeType* nextNode{ m_pGraph->GetNode(con->GetTo()) };
				float gCost{ currentNodeRecord.costSoFar + con->GetCost() };

				it = std::find_if(closedList.begin(), closedList.end(), [nextNode](const NodeRecord& record) { return nextNode == record.pNode; });
				if (it != closedList.end())
				{
					if ((*it).costSoFar > gCost)
					{
						closedList.erase(it);
					}
					else
					{
						continue;
					}
				}
				
				it = std::find_if(openList.begin(), openList.end(), [nextNode](const NodeRecord& record) { return nextNode == record.pNode; });
				if (it != openList.end())
				{
					if ((*it).costSoFar > gCost)
					{
						openList.erase(it);
					}
					else
					{
						continue;
					}
				}
				
				NodeRecord newNodeRecord;
				newNodeRecord.pNode = nextNode;
				newNodeRecord.pConnection = con;
				newNodeRecord.costSoFar = gCost;
				newNodeRecord.estimatedTotalCost = gCost + GetHeuristicCost(currentNodeRecord.pNode , pGoalNode);
				openList.push_back(newNodeRecord);
			}

			
			closedList.push_back(currentNodeRecord);
			openList.erase(std::remove(openList.begin(), openList.end(), currentNodeRecord));
		}

		while (currentNodeRecord.pNode != pStartNode)
		{
			path.push_back(currentNodeRecord.pNode);
			auto it{ std::find_if(closedList.begin(), closedList.end(), [currentNodeRecord, this](const NodeRecord& nodeRecord) {
				return m_pGraph->GetNode(currentNodeRecord.pConnection->GetFrom()) == nodeRecord.pNode;
				}) };
			currentNodeRecord = *it;
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}