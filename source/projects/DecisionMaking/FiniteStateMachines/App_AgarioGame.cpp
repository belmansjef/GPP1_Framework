#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"


//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;
App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pCustomAgent);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pConditions)
	{
		SAFE_DELETE(t);
	}

}

void App_AgarioGame::Start()
{
	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	// Common states
	WanderState* pWanderState = new WanderState();
	m_pStates.emplace_back(pWanderState);

	//Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		Blackboard* pBlackboard = CreateBlackboard(newAgent);

		FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackboard);
		newAgent->SetDecisionMaking(pStateMachine);

		m_pAgentVec.push_back(newAgent);
	}

	
	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
	Color customColor = Color{ 0.0f, 1.0f, 0.0f };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//1. Create and add the necessary blackboard data
	Blackboard* pBlackboard = CreateBlackboard(m_pCustomAgent);
	//2. Create the different agent states
	SeekFoodState* pSeekFood = new SeekFoodState();
	FleeTargetState* pFleeTarget = new FleeTargetState();
	PursuitTargetState* pPursuitTarget = new PursuitTargetState();
	m_pStates.emplace_back(pSeekFood);
	m_pStates.emplace_back(pFleeTarget);
	m_pStates.emplace_back(pPursuitTarget);

	//3. Create the conditions to transitions beetween those states
	FoodNearByCondition* pFoodNearBy = new FoodNearByCondition();
	FoodConsumedCondition* pFoodConsumed = new FoodConsumedCondition();
	BiggerAgentNearByCondition* pBiggerAgentNearBy = new BiggerAgentNearByCondition();
	NoBiggerAgentNearByCondition* pNoBiggerAgentNearBy = new NoBiggerAgentNearByCondition();
	SmallerAgentNearByCondition* pSmallerAgentNearBy = new SmallerAgentNearByCondition();
	SmallerAgentConsumedOrNoLongerSmallerCondition* pSmallerAgentConsumed = new SmallerAgentConsumedOrNoLongerSmallerCondition();
	m_pConditions.emplace_back(pFoodNearBy);
	m_pConditions.emplace_back(pFoodConsumed);
	m_pConditions.emplace_back(pBiggerAgentNearBy);
	m_pConditions.emplace_back(pNoBiggerAgentNearBy);
	m_pConditions.emplace_back(pSmallerAgentNearBy);
	m_pConditions.emplace_back(pSmallerAgentConsumed);

	//4. Create the finite state machine with a starting state and the blackboard
	FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackboard);

	//5. Add the transitions for the states to the state machine
	// stateMachine->AddTransition(startState, toState, condition)
	// startState: active state for which the transition will be checked
	// condition: if the Evaluate function returns true => transition will fire and move to the toState
	// toState: end state where the agent will move to if the transition fires
	
	// To flee
	pStateMachine->AddTransition(pWanderState, pFleeTarget, pBiggerAgentNearBy);
	pStateMachine->AddTransition(pSeekFood, pFleeTarget, pBiggerAgentNearBy);
	pStateMachine->AddTransition(pPursuitTarget, pFleeTarget, pBiggerAgentNearBy);

	// To pursuit
	pStateMachine->AddTransition(pWanderState, pPursuitTarget, pSmallerAgentNearBy);
	pStateMachine->AddTransition(pSeekFood, pPursuitTarget, pSmallerAgentNearBy);

	// To food
	pStateMachine->AddTransition(pWanderState, pSeekFood, pFoodNearBy);

	// To wander
	pStateMachine->AddTransition(pFleeTarget, pWanderState, pNoBiggerAgentNearBy);
	pStateMachine->AddTransition(pSeekFood, pWanderState, pFoodConsumed);
	pStateMachine->AddTransition(pPursuitTarget, pWanderState, pSmallerAgentConsumed);

	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pCustomAgent->SetDecisionMaking(pStateMachine);
	m_pCustomAgent->SetRenderBehavior(true);
}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pCustomAgent->CanBeDestroyed())
	{
		m_GameOver = true;

		//Update the other agents and food
		UpdateAgarioEntities(m_pFoodVec, deltaTime);
		UpdateAgarioEntities(m_pAgentVec, deltaTime);
		return;
	}
	//Update the custom agent
	m_pCustomAgent->Update(deltaTime);
	m_pCustomAgent->TrimToWorld(m_TrimWorldSize, false);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pCustomAgent->Render(deltaTime);
}

Blackboard* App_AgarioGame::CreateBlackboard(AgarioAgent* a)
{
	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("NearestFood", static_cast<AgarioFood*>(nullptr));
	pBlackboard->AddData("EnemyAgents", &m_pAgentVec);
	pBlackboard->AddData("NearestThreat", static_cast<AgarioAgent*>(nullptr));
	pBlackboard->AddData("FleeRadius", 15.f);
	pBlackboard->AddData("NearestTarget", static_cast<AgarioAgent*>(nullptr));
	return pBlackboard;
}

void App_AgarioGame::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
