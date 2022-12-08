/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return Elite::BehaviorState::Failure;

		pAgent->SetToWander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return Elite::BehaviorState::Failure;

		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Target", targetPos)) return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToAvadeAgent(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return Elite::BehaviorState::Failure;

		AgarioAgent* pEvadeAgent;
		if (!pBlackboard->GetData("AgentTarget", pEvadeAgent) || pEvadeAgent == nullptr) return Elite::BehaviorState::Failure;

		pAgent->SetToFlee(pEvadeAgent->GetPosition(), pAgent->GetRadius() + 25.f);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekAgent(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return Elite::BehaviorState::Failure;

		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Target", targetPos)) return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToPursuitAgent(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return Elite::BehaviorState::Failure;

		AgarioAgent* pTargetAgent;
		if (!pBlackboard->GetData("AgentTarget", pTargetAgent) || pTargetAgent == nullptr) return Elite::BehaviorState::Failure;

		pAgent->SetToPursuit(pTargetAgent);
	}
}


//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;

		std::vector<AgarioFood*>* pFoodVec;
		if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr || pFoodVec->empty()) return false;

		const float searchRadius{ pAgent->GetRadius() + 20.f };
		float closestDistSqr{ searchRadius * searchRadius };
		AgarioFood* pClosestFood{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		for (auto& pFood : *pFoodVec)
		{
			float distSqr{ pFood->GetPosition().DistanceSquared(agentPos) };

			if (distSqr < closestDistSqr)
			{
				closestDistSqr = distSqr;
				pClosestFood = pFood;
			}
		}

		if (pClosestFood != nullptr) 
		{
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}

		return false;
	}
	
	bool IsBiggerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;
		
		std::vector<AgarioAgent*>* pOtherAgents;
		if (!pBlackboard->GetData("AgentsVec", pOtherAgents) || pOtherAgents == nullptr || pOtherAgents->empty()) return false;

		const float fleeRadius{ pAgent->GetRadius() + 25.f };
		float closestDistSqr{ fleeRadius * fleeRadius };
		AgarioAgent* pClosestAgent{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, fleeRadius, Elite::Color{ 1.f, 0.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

		for (auto& pOtherAgent : *pOtherAgents)
		{
			if (pOtherAgent->GetRadius() <= pAgent->GetRadius() + 1) continue;

			float distSqr{ pOtherAgent->GetPosition().DistanceSquared(agentPos) - pOtherAgent->GetRadius() * pOtherAgent->GetRadius() };

			if (distSqr < closestDistSqr)
			{
				closestDistSqr = distSqr;
				pClosestAgent = pOtherAgent;
			}	
		}

		if (pClosestAgent != nullptr)
		{
			pBlackboard->ChangeData("AgentTarget", pClosestAgent);
			return true;
		}

		return false;
	}

	bool IsSmallerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;

		std::vector<AgarioAgent*>* pOtherAgents;
		if (!pBlackboard->GetData("AgentsVec", pOtherAgents) || pOtherAgents == nullptr || pOtherAgents->empty()) return false;

		const float agentRadius{ pAgent->GetRadius() };
		const float chaseRadius{ agentRadius + 30.f };
		float closestDistSqr{ chaseRadius * chaseRadius };
		AgarioAgent* pClosestAgent{ nullptr };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, chaseRadius, Elite::Color{ 0.f, 0.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());

		for (auto& pOtherAgent : *pOtherAgents)
		{
			if (pOtherAgent->GetRadius() + 1 >= pAgent->GetRadius()) continue;

			float distSqr{ pOtherAgent->GetPosition().DistanceSquared(agentPos) - pOtherAgent->GetRadius() * pOtherAgent->GetRadius()};

			if (distSqr < closestDistSqr)
			{
				closestDistSqr = distSqr;
				pClosestAgent = pOtherAgent;
			}
		}

		if (pClosestAgent != nullptr)
		{
			pBlackboard->ChangeData("Target", pClosestAgent->GetPosition());
			pBlackboard->ChangeData("AgentTarget", pClosestAgent);
			return true;
		}

		return false;
	}
}


#endif