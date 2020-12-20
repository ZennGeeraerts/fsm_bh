#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (!IsConnected())
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree 
		int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		int oddCount{};

		for (size_t i{}; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() & 1)) // kijkt of aantal node connections oneven is
			{
				++oddCount;
			}
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		else if ((oddCount == 2) && (nrOfNodes != 2))
		{
			return Eulerianity::semiEulerian;
		}

		// A connected graph with no odd nodes is Eulerian
		else
		{
			return Eulerianity::eulerian;
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		vector<T_NodeType*> path = vector<T_NodeType*>();

		// algorithm...
		int oddCount{};
		int oddNodeIndex{};

		for (size_t i{}; i < nrOfNodes; ++i)
		{
			if (graphCopy->IsNodeValid(i) && (graphCopy->GetNodeConnections(i).size() & 1)) // kijkt of aantal node connections oneven is
			{
				++oddCount;
				oddNodeIndex = i;
			}
		}

		int currentNode{};

		if (oddCount == 0)
		{
			currentNode = 0;
		}
		else if (oddCount == 2)
		{
			currentNode = oddNodeIndex;
		}
		else
		{
			return path;
		}

		vector<T_NodeType*> stack = vector<T_NodeType*>();

		do
		{
			if (graphCopy->GetNodeConnections(currentNode).size() == 0)
			{
				path.push_back(m_pGraph->GetNode(currentNode));
			
				currentNode = stack[stack.size() - 1]->GetIndex();
				stack.pop_back();
			}
			else
			{
				stack.push_back(graphCopy->GetNode(currentNode));
				auto nodeConnection{ graphCopy->GetNodeConnections(currentNode).front() };
				int neighborNode{ nodeConnection->GetTo() };
				graphCopy->RemoveConnection(currentNode, neighborNode);
				currentNode = neighborNode;
			}
		} 
		while ((stack.size() != 0) && (graphCopy->GetNodeConnections(currentNode).size() != 0));

		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	bool IsBridge(int node1, int node2)
	{
		return (graphCopy->GetNodeConnections(node1).size() == 1) || (graphCopy->GetNodeConnections(node1).size() == 1)
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* pConnection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(pConnection->GetTo()) && !visited[pConnection->GetTo()])
			{
				VisitAllNodesDFS(pConnection->GetTo(), visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		int nrOfNodes{ m_pGraph->GetNrOfNodes() };
		vector<bool> visited(nrOfNodes, false);

		// find a valid starting node that has connections
		int connectedIndex{ invalid_node_index };
		for (size_t i{}; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && m_pGraph->GetNodeConnections(i).size() != 0)
			{
				connectedIndex = i;
				break;
			}
		}

		// if no valid node could be found, return false
		if (connectedIndex == invalid_node_index)
		{
			return false;
		}

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIndex, visited);

		// if a node was never visited, this graph is not connected
		for (size_t i{}; i < nrOfNodes; ++i)
		{
			if (m_pGraph->IsNodeValid(i) && (visited[i] == false))
			{
				return false;
			}
		}

		return true;
	}

}