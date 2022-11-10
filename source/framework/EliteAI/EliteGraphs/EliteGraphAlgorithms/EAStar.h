#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

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
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		NodeRecord startRecord;
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.costSoFar = 0.f;
		startRecord.estimatedTotalCost = GetHeuristicCost(startRecord.pNode, pGoalNode);
		openList.emplace_back(startRecord);

		while (!openList.empty())
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());
			
			if (currentRecord.pNode == pGoalNode)
				break;

			for (auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				T_NodeType* pNextNode{ m_pGraph->GetNode(connection->GetTo()) };
				float gCost = currentRecord.costSoFar + 1 + connection->GetCost();

				bool isCheaperConnectionFound = false;

				// Check if connection already exists
				for (NodeRecord& existingRecord : closedList)
				{
					// Is current connection cheaper?
					if (existingRecord.pNode == pNextNode)
					{
						if (existingRecord.costSoFar < gCost)
						{
							isCheaperConnectionFound = true;
							break;
						}
						else
						{
							closedList.erase(std::remove(closedList.begin(), closedList.end(), existingRecord));
						}
					}
				}

				for (NodeRecord& existingRecord : openList)
				{
					// Is current connection cheaper?
					if (existingRecord.pNode == pNextNode)
					{
						if (existingRecord.costSoFar < gCost)
						{
							isCheaperConnectionFound = true;
							break;
						}
						else
						{
							openList.erase(std::remove(openList.begin(), openList.end(), existingRecord));
						}
					}
				}
				if (isCheaperConnectionFound) continue;

				// Create new node and add to open list
				NodeRecord record;
				record.pNode = pNextNode;
				record.costSoFar = gCost;
				record.estimatedTotalCost = gCost + GetHeuristicCost(pNextNode, pGoalNode);
				record.pConnection = connection;
				openList.push_back(record);
			} // Connections for-loop

			// Move current record from open to closed list
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);
		} // While-loop

		if(currentRecord.pNode != pGoalNode)
			return std::vector<T_NodeType*>();

		while (currentRecord.pNode != pStartNode)
		{
			path.emplace_back(currentRecord.pNode);
			for (NodeRecord& record : closedList)
			{
				if (record.pNode->GetIndex() == currentRecord.pConnection->GetFrom())
				{
					currentRecord = record;
					break;
				}
			}
		}

		path.emplace_back(pStartNode);
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