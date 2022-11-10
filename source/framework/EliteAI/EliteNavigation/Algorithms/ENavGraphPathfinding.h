#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			Triangle *startTriangle = nullptr, *endTriangle = nullptr;
			for (const auto tri : pNavGraph->GetNavMeshPolygon()->GetTriangles())
			{
				if (PointInTriangle(startPos, tri->p1, tri->p2, tri->p3))
				{
					startTriangle = tri;
				}
				if (PointInTriangle(endPos, tri->p1, tri->p2, tri->p3))
				{
					endTriangle = tri;
				}

				// we got start and end triangle, stop looping triangles
				if (startTriangle != nullptr && endTriangle != nullptr) break;
			}

			if (startTriangle == nullptr || endTriangle == nullptr) return finalPath;
		
			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}
			
			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto pNavGraphCopy = pNavGraph->Clone();
			
			//Create extra node for the Start Node (Agent's position)
			NavGraphNode* startNode = new NavGraphNode{ pNavGraphCopy->GetNextFreeNodeIndex(), -1, startPos};
			pNavGraphCopy->AddNode(startNode);
			for (const int lineIndex : startTriangle->metaData.IndexLines)
			{
				int nodeIndex = pNavGraph->GetNodeIdxFromLineIdx(lineIndex);
				if (nodeIndex == invalid_node_index) continue;
				const auto lineNode = pNavGraphCopy->GetNode(nodeIndex);
				pNavGraphCopy->AddConnection(new GraphConnection2D{nodeIndex, startNode->GetIndex(), Distance(lineNode->GetPosition(), startNode->GetPosition())});
			}

			//Create extra node for the endNode
			NavGraphNode* endNode = new NavGraphNode{ pNavGraphCopy->GetNextFreeNodeIndex(), -1, endPos };
			pNavGraphCopy->AddNode(endNode);
			for (const auto lineIndex : endTriangle->metaData.IndexLines)
			{
				int nodeIndex = pNavGraph->GetNodeIdxFromLineIdx(lineIndex);
				if (nodeIndex == invalid_node_index) continue;
				const auto lineNode = pNavGraphCopy->GetNode(nodeIndex);
				pNavGraphCopy->AddConnection(new GraphConnection2D{ nodeIndex, endNode->GetIndex(), Distance(lineNode->GetPosition(), endNode->GetPosition()) });
			}

			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(pNavGraphCopy.get(), Elite::HeuristicFunctions::Chebyshev);
			std::vector<Elite::NavGraphNode*> m_vPath = pathfinder.FindPath(startNode, endNode);

			for (const auto node : m_vPath)
			{
				finalPath.emplace_back(node->GetPosition());
			}
			
			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			//m_Portals = SSFA::FindPortals(nodes, m_pNavGraph->GetNavMeshPolygon());
			//finalPath = SSFA::OptimizePortals(m_Portals);

			return finalPath;
		}
	};
}
