//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if(pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, {0.f,1.f,0.f});
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const Elite::Vector2 fromTarget = pAgent->GetPosition() - m_Target.Position;
	const float distanceFromTarget = fromTarget.Magnitude();
	if(distanceFromTarget > m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0.f,1.f,0.f });
	}

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const float arrivalRadius = 1.f;
	const float slowRadius = 15.f;

	Elite::Vector2 toTarget = m_Target.Position - pAgent->GetPosition();
	const float distance = toTarget.Magnitude();

	if (distance < arrivalRadius)
	{
		steering.LinearVelocity = Elite::Vector2{0.f, 0.f};
		return steering;
	}

	auto velocity = toTarget;
	velocity.Normalize();

	if (distance < slowRadius)
		velocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius);
	else
		velocity *= pAgent->GetMaxLinearSpeed();

	steering.LinearVelocity = velocity;


	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 0.f,1.f,0.f });
	}

	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(false);

	Elite::Vector2 dir_vector = m_Target.Position - pAgent->GetPosition();
	dir_vector.Normalize();

	const float target_angle = atan2f(dir_vector.y, dir_vector.x);
	const float agent_angle = pAgent->GetRotation();
	const float delta_angle = target_angle - agent_angle;

	if (abs(target_angle - agent_angle) <= m_AngleError)
	{
		return steering;
	}

	steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	if (delta_angle < 0 || delta_angle > static_cast<float>(M_PI))
	{
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		const float x = cosf(agent_angle);
		const float y = sinf(agent_angle);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), {x, y}, 5.f, { 0.f,1.f,0.f });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), dir_vector, 5.f, { 0.f,1.f,0.f });
	}

	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	m_WanderAngle += Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange);
	const Elite::Vector2 circle_center = pAgent->GetPosition() + (pAgent->GetDirection() * m_OffsetDistance);
	const Elite::Vector2 desired_location = { cosf(m_WanderAngle) * m_Radius + circle_center.x, sinf(m_WanderAngle) * m_Radius + circle_center.y };

	m_Target.Position = desired_location;

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetDirection(), m_OffsetDistance, {0.f, 1.f, 0.f});
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desired_location - pAgent->GetPosition(), Distance(pAgent->GetPosition(), desired_location), { 0.f, 1.f, 0.f });
		DEBUGRENDERER2D->DrawCircle(circle_center, m_Radius, { 0.f, 1.f, 0.f }, 0);
		DEBUGRENDERER2D->DrawCircle(desired_location, 0.35f, { 1.f, 0.f, 0.f }, 0);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}
