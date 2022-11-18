#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

#pragma region States
void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return;

	std::cout << "Wandering\n";
	pAgent->SetToWander();
}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return;

	AgarioFood* nearestFood;
	if (!pBlackboard->GetData("NearestFood", nearestFood) || nearestFood == nullptr) return;

	std::cout << "Seeking Food\n";
	pAgent->SetToSeek(nearestFood->GetPosition());
}

void FSMStates::FleeTargetState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return;

	AgarioAgent* nearestThreat;
	if (!pBlackboard->GetData("NearestThreat", nearestThreat) || nearestThreat == nullptr) return;

	float fleeRadius;
	if (!pBlackboard->GetData("FleeRadius", fleeRadius) || fleeRadius == 0.f) return;

	std::cout << "Evading Threat\n";
	pAgent->SetToFlee(nearestThreat->GetPosition(), fleeRadius);
}

void FSMStates::FleeTargetState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return;

	AgarioAgent* nearestThreat;
	if (!pBlackboard->GetData("NearestThreat", nearestThreat) || nearestThreat == nullptr) return;

	float fleeRadius;
	if (!pBlackboard->GetData("FleeRadius", fleeRadius) || fleeRadius == 0.f) return;

	pAgent->SetToFlee(nearestThreat->GetPosition(), fleeRadius);
}

void FSMStates::PursuitTargetState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return;

	AgarioAgent* nearestTarget;
	if (!pBlackboard->GetData("NearestTarget", nearestTarget) || nearestTarget == nullptr) return;

	std::cout << "Chasing target\n";
	pAgent->SetToSeek(nearestTarget->GetPosition());
}

void FSMStates::PursuitTargetState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return;

	AgarioAgent* nearestTarget;
	if (!pBlackboard->GetData("NearestTarget", nearestTarget) || nearestTarget == nullptr) return;

	pAgent->SetToSeek(nearestTarget->GetPosition());
}
#pragma endregion

#pragma region Conditions
bool FoodNearByCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;
	if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr) return false;

	const float radius{ 20.f + pAgent->GetRadius() };
	const Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
	{
		float dist1 = pFood1->GetPosition().DistanceSquared(agentPos);
		float dist2 = pFood2->GetPosition().DistanceSquared(agentPos);

		return dist1 < dist2;
	};

	auto closestElementIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), isCloser);
	if (closestElementIt == pFoodVec->end()) return false;

	AgarioFood* nearestFood = *closestElementIt;
	if (nearestFood->GetPosition().DistanceSquared(agentPos) <= radius * radius)
	{
		pBlackboard->ChangeData("NearestFood", nearestFood);
		return true;
	}

	return false;
}

bool FSMConditions::NoFoodNearByCondtion::Evaluate(Elite::Blackboard* pBlackboard) const
{
	return !FoodNearByCondition::Evaluate(pBlackboard);
}

bool FSMConditions::BiggerAgentNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;

	std::vector<AgarioAgent*>* pAgentsVec;
	if (!pBlackboard->GetData("EnemyAgents", pAgentsVec) || pAgentsVec == nullptr) return false;

	const float agentRadius{ pAgent->GetRadius() };
	const float fleeRadius{ 25.f + agentRadius };
	const Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, fleeRadius, Color{ 1.f, 0.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser = [agentPos, agentRadius](AgarioAgent* pAgent1, AgarioAgent* pAgent2)
	{
		if (pAgent1->GetRadius() <= agentRadius + 1) return false;

		float dist1 = pAgent1->GetPosition().DistanceSquared(agentPos);
		float dist2 = pAgent2->GetPosition().DistanceSquared(agentPos);

		return dist1 < dist2;
	};

	auto closestElementIt = std::min_element(pAgentsVec->begin(), pAgentsVec->end(), isCloser);
	if (closestElementIt == pAgentsVec->end()) return false;

	AgarioAgent* nearestThreat = *closestElementIt;
	if (nearestThreat->GetRadius() <= agentRadius + 1) return false;

	if (nearestThreat->GetPosition().DistanceSquared(agentPos) <= fleeRadius * fleeRadius)
	{
		pBlackboard->ChangeData("NearestThreat", nearestThreat);
		pBlackboard->ChangeData("FleeRadius", fleeRadius);
		return true;
	}

	return false;
}

bool FSMConditions::NoBiggerAgentNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	return !BiggerAgentNearByCondition::Evaluate(pBlackboard);
}

bool FSMConditions::FoodConsumedCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioFood* nearestFood;
	if (!pBlackboard->GetData("NearestFood", nearestFood) || nearestFood == nullptr || nearestFood->CanBeDestroyed()) return true;

	return false;
}

bool FSMConditions::SmallerAgentNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;

	std::vector<AgarioAgent*>* pAgentsVec;
	if (!pBlackboard->GetData("EnemyAgents", pAgentsVec) || pAgentsVec == nullptr) return false;

	const float agentRadius{ pAgent->GetRadius() };
	const float pursuitRadius{ 20.f + agentRadius };
	const Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, pursuitRadius, Color{ 0.f, 0.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser = [agentPos, agentRadius](AgarioAgent* pAgent1, AgarioAgent* pAgent2)
	{
		if (pAgent1->GetRadius() >= agentRadius + 1 || pAgent2->GetRadius() >= agentRadius + 1) return false;

		float dist1 = pAgent1->GetPosition().DistanceSquared(agentPos);
		float dist2 = pAgent2->GetPosition().DistanceSquared(agentPos);

		return dist1 < dist2;
	};

	auto closestElementIt = std::min_element(pAgentsVec->begin(), pAgentsVec->end(), isCloser);
	if (closestElementIt == pAgentsVec->end()) return false;

	AgarioAgent* nearestTarget = *closestElementIt;
	if (nearestTarget->GetRadius() >= agentRadius + 1) return false;

	if (nearestTarget->GetPosition().DistanceSquared(agentPos) <= pursuitRadius * pursuitRadius)
	{
		pBlackboard->ChangeData("NearestTarget", nearestTarget);
		return true;
	}

	return false;
}

bool FSMConditions::NoSmallerAgentNearByCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	return !SmallerAgentNearByCondition::Evaluate(pBlackboard);
}

bool FSMConditions::SmallerAgentConsumedOrNoLongerSmallerCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr) return false;

	AgarioAgent* nearestTarget;
	if (!pBlackboard->GetData("NearestTarget", nearestTarget) || nearestTarget == nullptr || nearestTarget->CanBeDestroyed() || nearestTarget->GetRadius() >= pAgent->GetRadius() + 1) return true;

	return false;
}
#pragma endregion
