#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

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
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_CellWidth(width / static_cast<float>(cols))
	, m_CellHeight(height / static_cast<float>(rows))
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	for (int x{0}; x < m_NrOfRows; ++x)
	{
		for (int y{0}; y < m_NrOfCols; ++y)
		{
			m_Cells.emplace_back(Cell(m_CellWidth * x, m_CellHeight * y, m_CellWidth, m_CellHeight));
		}
	}
}
	
void CellSpace::AddAgent(SteeringAgent* agent)
{
	const int index{ Elite::randomInt(m_Cells.size()) };

	
	m_Cells[0].agents.emplace_back(agent);
	agent->SetPosition(m_Cells[index].GetRectPoints()[0]);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	const int currentPosIndex{ PositionToIndex(agent->GetPosition()) };
	const int oldPosIndex{ PositionToIndex(oldPos) };

	if (currentPosIndex == oldPosIndex) return;

	m_Cells[oldPosIndex].agents.remove(agent);
	m_Cells[currentPosIndex].agents.emplace_back(agent);
}

int CellSpace::RegisterNeighbors(SteeringAgent* pAgent, float queryRadius, std::vector<SteeringAgent*>& neighbors)
{
	RegisterNeighbors(pAgent, queryRadius);
	neighbors = m_Neighbors;
	return m_NrOfNeighbors;
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	m_NrOfNeighbors = 0;

	const Elite::Vector2 agentPos{ agent->GetPosition() };
	const Elite::Rect radiusBB
	{
		{agentPos.x - queryRadius, agentPos.y - queryRadius},
		queryRadius,
		queryRadius
	};

	for(const Cell& cell : m_Cells)
	{
		if(IsOverlapping(radiusBB, cell.boundingBox))
		{
			for(SteeringAgent* neighbor : cell.agents)
			{
				if (neighbor == agent) continue;

				if(DistanceSquared(agentPos, neighbor->GetPosition()) < Elite::Square(queryRadius))
				{
					m_Neighbors[m_NrOfNeighbors] = neighbor;
					++m_NrOfNeighbors;
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells(const SteeringAgent* pAgent, float queryRadius) const
{
	const Elite::Vector2 agentPos{ pAgent->GetPosition() };
	const Elite::Rect radiusBB
	{
		{agentPos.x - queryRadius, agentPos.y - queryRadius},
		queryRadius,
		queryRadius
	};

	for (Cell _cell : m_Cells)
	{
		auto* poly = new Elite::Polygon(_cell.GetRectPoints());
		std::string s = std::to_string(_cell.agents.size());

		if (IsOverlapping(radiusBB, _cell.boundingBox))
			DEBUGRENDERER2D->DrawPolygon(poly, {0.f, 0.f, 1.f}, 0.f);
		else
			DEBUGRENDERER2D->DrawPolygon(poly, { 1.f, 0.f, 0.f }, 1.f);

		DEBUGRENDERER2D->DrawString(
			Elite::Vector2
			(
				_cell.boundingBox.bottomLeft.x + _cell.boundingBox.width / 4.f,
				_cell.boundingBox.bottomLeft.y + _cell.boundingBox.height
			),
			s.c_str()
		);
		delete poly;
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{;
	for(int i{0}; i < m_Cells.size(); ++i)
	{
		if(
			pos.x >= m_Cells[i].boundingBox.bottomLeft.x &&
			pos.x <= m_Cells[i].boundingBox.bottomLeft.x + m_Cells[i].boundingBox.width &&
			pos.y >= m_Cells[i].boundingBox.bottomLeft.y &&
			pos.y <= m_Cells[i].boundingBox.bottomLeft.y + m_Cells[i].boundingBox.height
			)
		{
			return i;
		}
	}
	return 0;
}