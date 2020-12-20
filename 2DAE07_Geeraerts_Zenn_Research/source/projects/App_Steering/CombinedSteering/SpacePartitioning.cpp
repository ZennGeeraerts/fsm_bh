#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities, bool* pCanRenderDebug)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
	, m_CellWidth{ 0.f }
	, m_CellHeight{ 0.f }
	, m_pCanRenderDebug{ pCanRenderDebug }
{
	m_CellWidth = m_SpaceWidth / m_NrOfCols;
	m_CellHeight = m_SpaceHeight / m_NrOfRows;

	for (int i{}; i < m_NrOfRows; ++i)
	{
		for (int j{}; j < m_NrOfCols; ++j)
		{
			m_Cells.push_back(Cell{ (m_CellWidth * j) - (width / 2), (m_CellHeight * i) - (height / 2), m_CellWidth, m_CellHeight });
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	Elite::Vector2 agentPos{ agent->GetPosition() };
	int cellIndex{ PositionToIndex(agentPos) };
	m_Cells[cellIndex].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	int oldCellIndex{ PositionToIndex(oldPos) };
	int newCellIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldCellIndex != newCellIndex)
	{
		m_Cells[oldCellIndex].agents.remove(agent);
		m_Cells[newCellIndex].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius)
{
	m_NrOfNeighbors = 0;
	Elite::Rect rect;
	rect.bottomLeft = Elite::Vector2{ pos.x - queryRadius / 2, pos.y - queryRadius / 2 };
	rect.width = queryRadius;
	rect.height = queryRadius;

	for (int i{}; i < m_NrOfCols * m_NrOfRows; ++i)
	{
		std::vector<Elite::Vector2> cellRectPoints{ m_Cells[i].GetRectPoints() };
		Elite::Rect cellRect;
		cellRect.bottomLeft = cellRectPoints[0];
		cellRect.width = cellRectPoints[3].x - cellRectPoints[0].x;
		cellRect.height = cellRectPoints[1].y - cellRectPoints[0].y;

		if (Elite::IsOverlapping(rect, cellRect))
		{
			for (std::list<SteeringAgent*>::iterator it{ m_Cells[i].agents.begin() }; it != m_Cells[i].agents.end(); ++it)
			{
				if (Elite::Distance((*it)->GetPosition(), pos) <= queryRadius)
				{
					if (m_NrOfNeighbors < (m_Neighbors.capacity() - 1))
					{
						m_Neighbors[m_NrOfNeighbors] = *it;
						++m_NrOfNeighbors;
					}
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	if (*m_pCanRenderDebug)
	{
		for (int i{}; i < m_NrOfCols * m_NrOfRows; ++i)
		{
			DEBUGRENDERER2D->DrawPolygon(&m_Cells[i].GetRectPoints()[0], 4, Elite::Color{ 1, 0, 0 }, DEBUGRENDERER2D->NextDepthSlice());

			if (m_Cells[i].agents.size() > 0)
			{
				DEBUGRENDERER2D->DrawSolidPolygon(&m_Cells[i].GetRectPoints()[0], 4, Elite::Color{ 0, 1, 0 }, DEBUGRENDERER2D->NextDepthSlice());
			}
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	Elite::Vector2 levelPos{ pos.x + m_CellWidth * m_NrOfCols / 2.f, pos.y + m_CellHeight * m_NrOfRows / 2.f };
	int xValue{ int(levelPos.x / m_CellWidth) };
	int yValue{ int(levelPos.y / m_CellHeight) };

	return (yValue * m_NrOfCols + xValue) % (m_NrOfCols * m_NrOfRows);
}