#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/,
	float worldSize /*= 100.f*/,
	SteeringAgent* pAgentToEvade /*= nullptr*/,
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{ 0 }
{
	m_Agents.resize(m_FlockSize);

	// TODO: initialize the flock and the memory pool

	m_pSeekBehavior = new Seek();
	m_pSeparationBehavior = new Separation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pWanderBehavior = new Wander();
	m_pFleeBehavior = new Flee();

	m_pBlendedSteering = new BlendedSteering({
		{m_pSeekBehavior, 0.5f},
		{m_pCohesionBehavior, 0.5f},
		{m_pVelMatchBehavior, 0.5f},
		{m_pWanderBehavior, 0.5f},
		});

	m_pPrioritySteering = new PrioritySteering({ m_pFleeBehavior, m_pBlendedSteering });

	for (int i{ 0 }; i < m_FlockSize; ++i)
	{
		m_Agents[i] = new SteeringAgent();
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(15.f);
		m_Agents[i]->SetMass(0.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetBodyColor({ 1, 0, 0 });
	}

	m_Neighbors.resize(m_FlockSize - 1);
}

Flock::~Flock()
{
	// TODO: clean up any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();

	for(auto pNeighborAgent: m_Neighbors)
	{
		SAFE_DELETE(pNeighborAgent);
	}
	m_Neighbors.clear();
}

void Flock::Update(float deltaT)
{
	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world
	for(auto pAgent : m_Agents)
	{
		RegisterNeighbors(pAgent);
		pAgent->Update(deltaT);
		pAgent->TrimToWorld(m_WorldSize);
	}
}

void Flock::Render(float deltaT)
{
	// TODO: render the flock
	for (const auto pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// TODO: Implement
	m_NrOfNeighbors = 0;
	const Elite::Vector2 agentPos = pAgent->GetPosition();

	for (const auto pOtherAgent : m_Agents)
	{
		if (pOtherAgent == pAgent) continue;

		if(DistanceSquared(agentPos, pOtherAgent->GetPosition()) <= Square(m_NeighborhoodRadius))
		{
			m_Neighbors.push_back(pOtherAgent);
			++m_NrOfNeighbors;
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 combinedPos{};
	for (int i{0}; i < m_NrOfNeighbors; ++i)
	{
		combinedPos += m_Neighbors[i]->GetPosition();
	}

	return combinedPos / static_cast<float>(m_NrOfNeighbors);
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	// TODO: Implement

	Elite::Vector2 combinedVelocity{};
	for (int i{ 0 }; i < m_NrOfNeighbors; ++i)
	{
		combinedVelocity += m_Neighbors[i]->GetLinearVelocity();
	}

	return combinedVelocity / static_cast<float>(m_NrOfNeighbors);
}

void Flock::SetTarget_Seek(TargetData target)
{
	// TODO: Set target for seek behavior
	for (const auto pAgent : m_Agents)
	{
		pAgent->GetSteeringBehavior()->SetTarget(target);
	}
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
