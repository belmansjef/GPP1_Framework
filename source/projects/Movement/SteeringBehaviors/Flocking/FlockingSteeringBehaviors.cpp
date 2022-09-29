#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	m_Target.Position = m_pFlock->GetAverageNeighborPos();
	return SteeringOutput();
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	if (m_pFlock->GetNrOfNeighbors() == 0) return SteeringOutput();

	SteeringOutput steering{};
	Elite::Vector2 pushForce{};
	const std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
	const int nrOfNeighbors{ m_pFlock->GetNrOfNeighbors() };

	for (int i {0}; i < nrOfNeighbors; ++i)
	{
		pushForce += neighbors[i]->GetPosition() - pAgent->GetPosition();
	}

	pushForce /= nrOfNeighbors;
	pushForce *= -1.f;

	steering.LinearVelocity = pushForce;

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity() * pAgent->GetMaxLinearSpeed();
	return steering;
}
