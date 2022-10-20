//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;
using namespace std;
//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(m_Graph2D);
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	m_Graph2D = new Graph2D<GraphNode2D, GraphConnection2D>(false);
	m_Graph2D->AddNode(new GraphNode2D(0, {20, 30}));
	m_Graph2D->AddNode(new GraphNode2D(1, {-10, -10}));
	m_Graph2D->AddConnection(new GraphConnection2D(0, 1));
}

void App_GraphTheory::Update(float deltaTime)
{
	m_GraphEditor.UpdateGraph(m_Graph2D);
	m_Graph2D->SetConnectionCostsToDistance();

	auto eulerFinder = EulerianPath<GraphNode2D, GraphConnection2D>(m_Graph2D);
	Eulerianity eulerianity = eulerFinder.IsEulerian();
	auto path = eulerFinder.FindPath(eulerianity);

	switch (eulerianity)
	{
	case Elite::Eulerianity::notEulerian:
		cout << "Not eulerian \n";
		for (auto p : m_Graph2D->GetAllNodes())
		{
			p->SetColor({ 1.f, 1.f, 1.f });
		}
		break;
	case Elite::Eulerianity::semiEulerian:
		cout << "Semi eulerian\n";
		break;
	case Elite::Eulerianity::eulerian:
		cout << "Eulerian\n";
		break;
	default:
		break;
	}

	cout << path.size() << endl;
	const float gradientStep{ 1.f / path.size() };
	for (int i = 0; i < path.size(); i++)
	{
		path[i]->SetColor({1.f - gradientStep * i, 1.f - gradientStep * i, 1.f - gradientStep * i});
		cout << path[i]->GetIndex() << " > ";
	}
	cout << "\n";

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
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_GraphTheory::Render(float deltaTime) const
{
	m_GraphRenderer.RenderGraph(m_Graph2D, true, true);
}
