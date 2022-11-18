/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState final : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState final : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeTargetState final : public Elite::FSMState
	{
	public:
		void OnEnter(Elite::Blackboard* pBlackboard) override;
		void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};

	class PursuitTargetState final : public Elite::FSMState
	{
	public:
		void OnEnter(Elite::Blackboard* pBlackboard) override;
		void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	};
}


//-----------------
//---TRANSITIONS---
//-----------------
namespace FSMConditions
{
	class FoodNearByCondition : public Elite::FSMCondition
	{
	public:
		FoodNearByCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoFoodNearByCondtion final : public FoodNearByCondition
	{
	public:
		NoFoodNearByCondtion() : FoodNearByCondition() {};
		bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class FoodConsumedCondition final : public Elite::FSMCondition
	{
	public:
		FoodConsumedCondition() : FSMCondition() {};
		bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class BiggerAgentNearByCondition : public Elite::FSMCondition
	{
	public:
		BiggerAgentNearByCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoBiggerAgentNearByCondition final : public BiggerAgentNearByCondition
	{
	public:
		NoBiggerAgentNearByCondition() : BiggerAgentNearByCondition() {};
		bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallerAgentNearByCondition : public Elite::FSMCondition
	{
	public:
		SmallerAgentNearByCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoSmallerAgentNearByCondition final : public SmallerAgentNearByCondition
	{
	public:
		NoSmallerAgentNearByCondition() : SmallerAgentNearByCondition() {};
		bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallerAgentConsumedOrNoLongerSmallerCondition final : public Elite::FSMCondition
	{
	public:
		SmallerAgentConsumedOrNoLongerSmallerCondition() : FSMCondition() {};
		bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}
#endif