#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	std::vector<Line*> pLines{ m_pNavMeshPolygon->GetLines() };
	size_t lineSize{ pLines.size() };

	for (size_t i{}; i < lineSize; ++i)
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(i).size() > 1)
		{
			Elite::Vector2 pos{ (pLines[i]->p1 + pLines[i]->p2) / 2 };
			AddNode(new NavGraphNode{ GetNrOfActiveNodes(), pLines[i]->index, pos });
		}
	}
	
	//2. Create connections now that every node is created
	std::vector<Triangle*> pTriangles{ m_pNavMeshPolygon->GetTriangles() };
	size_t triangleSize{ pTriangles.size() };

	for (size_t i{}; i < triangleSize; ++i)
	{
		std::vector<int> validNodes{};

		for (size_t j{}; j < pTriangles[i]->metaData.IndexLines.size(); ++j)
		{
			
			int nodeIdx{ GetNodeIdxFromLineIdx(pTriangles[i]->metaData.IndexLines[j]) };
			if (nodeIdx != invalid_node_index)
			{
				validNodes.push_back(GetNodeIdxFromLineIdx(pTriangles[i]->metaData.IndexLines[j]));
			}
			
		}

		if (validNodes.size() == 2)
		{
			AddConnection(new GraphConnection2D{ validNodes[0], validNodes[1] });
		}
		else if (validNodes.size() == 3)
		{
			AddConnection(new GraphConnection2D{ validNodes[0], validNodes[1] });
			AddConnection(new GraphConnection2D{ validNodes[1], validNodes[2] });
			AddConnection(new GraphConnection2D{ validNodes[2], validNodes[0] });
		}
	}
	
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

