#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const Elite::Vector2 desiredPos{m_pFlock->GetAverageNeighborPos()};
	const Elite::Vector2 desiredVel{ desiredPos - pAgent->GetPosition() };

	steering.LinearVelocity = desiredVel.GetNormalized() * pAgent->GetMaxLinearSpeed();

	// m_Target.Position = m_pFlock->GetAverageNeighborPos();
	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	if (m_pFlock->GetNrOfNeighbors() == 0) return SteeringOutput();

	SteeringOutput steering{};
	Elite::Vector2 totalForce{};
	const std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };
	const int nrOfNeighbors{ m_pFlock->GetNrOfNeighbors() };

	for (int i {0}; i < nrOfNeighbors; ++i)
	{
		Elite::Vector2 pushForce = neighbors[i]->GetPosition() - pAgent->GetPosition();
		pushForce /= pushForce.MagnitudeSquared();
		totalForce += pushForce;
	}

	totalForce *= -1.f;
	totalForce = totalForce.GetNormalized() * pAgent->GetMaxLinearSpeed();
	steering.LinearVelocity = totalForce;

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborVel() * pAgent->GetMaxLinearSpeed();
	return steering;
}
