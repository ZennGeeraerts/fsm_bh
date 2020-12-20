#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	Elite::Vector2* pMousePosition,
	Elite::Vector2* pOtherAgentPosition,
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
	, m_pMousePosition{ pMousePosition }
	, m_pOtherAgentPosition{ pOtherAgentPosition }
{
	m_pCellSpace = new CellSpace{ worldSize * 2, worldSize * 2, m_NrOfRows, m_NrOfColls, flockSize, &m_CanDebugRender };
	m_OldPositions.resize(flockSize);

	m_Agents.resize(flockSize);
	m_Neighbors.resize(flockSize);
	m_pSeek = new Seek{};
	m_pSeek->SetTarget(*pMousePosition);
	m_pWander = new Wander{};
	m_pEvade = new Evade{};
	m_pEvade->SetTarget((*m_pOtherAgentPosition));
	m_pCohesion = new Cohesion{ this };
	m_pSeperation = new Seperation{ this, m_NeighborhoodRadius };
	m_pAlignment = new Alignment{ this };
	m_pBlendedSteering = new BlendedSteering{ std::vector<BlendedSteering::WeightedBehavior>{ { m_pSeek, 0.5f}, { m_pWander, 0.5f },
	{ m_pCohesion, 0.5f }, { m_pSeperation, 0.5f }, { m_pAlignment, 0.5f } } };
	m_pPrioritySteering = new PrioritySteering{ { m_pEvade, m_pBlendedSteering } };

	const float otherAgentSpawnRadius{ 20.f };
	for (int i{}; i < flockSize; ++i)
	{
		m_Agents[i] = new SteeringAgent{};
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(15.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(1.f);

		do
		{
			m_Agents[i]->SetPosition(Elite::Vector2{ rand() % int(worldSize * 2) - worldSize, rand() % int(worldSize * 2) - worldSize });
		} while (Distance(m_Agents[i]->GetPosition(), (*m_pOtherAgentPosition)) < otherAgentSpawnRadius);

		m_pCellSpace->AddAgent(m_Agents[i]);
	}
}

Flock::~Flock()
{
	for (int i{}; i < m_FlockSize; ++i)
	{
		SAFE_DELETE(m_Agents[i]);
	}

	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeperation);
	SAFE_DELETE(m_pAlignment);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{
	m_pSeek->SetTarget((*m_pMousePosition));
	m_pEvade->SetTarget(m_pAgentToEvade->GetPosition());

	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	Elite::Vector2 agentPos;

	for (int i{}; i < m_FlockSize; ++i)
	{
		agentPos = m_Agents[i]->GetPosition();

		if (m_IsUsingPartitioning)
		{
			m_pCellSpace->UpdateAgentCell(m_Agents[i], m_OldPositions[i]);
			m_OldPositions[i] = agentPos;

			m_pCellSpace->RegisterNeighbors(agentPos, m_QueryRadius);
		}
		else
		{
			RegisterNeighbors(m_Agents[i]);
		}

		m_Agents[i]->Update(deltaT);

		m_Agents[i]->TrimToWorld(m_WorldSize);
	}

	UpdateAndRenderUI();
}

void Flock::Render(float deltaT)
{
	/*for (int i{}; i < m_FlockSize; ++i)
	{
		m_Agents[i]->Render(deltaT);
	}*/

	if (m_IsUsingPartitioning)
	{
		m_pCellSpace->RenderCells();
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

	ImGui::Checkbox("Enable spatial partitioning", &m_IsUsingPartitioning);
	ImGui::Spacing();

	ImGui::SliderFloat("Neighborhood radius", &m_NeighborhoodRadius, 0.f, 50.0f, "%.0");
	ImGui::SliderFloat("Query radius", &m_QueryRadius, 0.f, 50.0f, "%.0");
	ImGui::Spacing();

	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
	ImGui::Spacing;

	// Implement checkboxes and sliders here
	ImGui::Text("Behavior Weights");
	ImGui::Spacing();
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.0f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.0f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.0f, "%.2");
	ImGui::SliderFloat("Seperate", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.0f, "%.2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.0f, "%.2");
	ImGui::Spacing();

	for (int i{}; i < m_FlockSize; ++i)
	{
		ImGui::PushID(i);
		char headerName[100];
		snprintf(headerName, sizeof(headerName), "ACTOR %i", i);

		bool isChecked = m_Agents[i]->CanRenderBehavior();
		ImGui::Checkbox("Render Debug", &isChecked);
		m_Agents[i]->SetRenderBehavior(isChecked);

		ImGui::PopID();
	}

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;

	for (int i{}; i < m_FlockSize; ++i)
	{
		if (m_Agents[i] != pAgent)
		{
			if (Elite::Distance(m_Agents[i]->GetPosition(), pAgent->GetPosition()) <= m_NeighborhoodRadius)
			{
				if (m_NrOfNeighbors < (m_Neighbors.capacity() - 1))
				{
					m_Neighbors[m_NrOfNeighbors] = m_Agents[i];
					++m_NrOfNeighbors;
				}
			}
		}
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, Elite::Color{ 1, 1, 0 }, 0.40f);
	}
}

int Flock::GetNrOfNeighbors() const
{
	if (m_IsUsingPartitioning)
	{
		return m_pCellSpace->GetNrOfNeighbors();
	}
	else
	{
		return m_NrOfNeighbors;
	}
}

const vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	if (m_IsUsingPartitioning)
	{
		return m_pCellSpace->GetNeighbors();
	}
	else
	{
		return m_Neighbors;
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 totalNeighborPos{};
	std::vector<SteeringAgent*> pNeighbors;
	int nrOfNeighbors;

	if (m_IsUsingPartitioning)
	{
		pNeighbors = m_pCellSpace->GetNeighbors();
		nrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
	}
	else
	{
		pNeighbors = m_Neighbors;
		nrOfNeighbors = m_NrOfNeighbors;
	}

	for (int i{}; i < nrOfNeighbors; ++i)
	{
		totalNeighborPos += pNeighbors[i]->GetPosition();
	}

	if (nrOfNeighbors > 0)
	{
		return totalNeighborPos / float(nrOfNeighbors);
	}
	else
	{
		return Elite::Vector2{ 0, 0 };
	}
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 totalNeighborVelocity{};
	std::vector<SteeringAgent*> pNeighbors;
	int nrOfNeighbors;

	if (m_IsUsingPartitioning)
	{
		pNeighbors = m_pCellSpace->GetNeighbors();
		nrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
	}
	else
	{
		pNeighbors = m_Neighbors;
		nrOfNeighbors = m_NrOfNeighbors;
	}

	for (int i{}; i < nrOfNeighbors; ++i)
	{
		totalNeighborVelocity += pNeighbors[i]->GetLinearVelocity();
	}

	if (nrOfNeighbors > 0)
	{
		return totalNeighborVelocity / float(nrOfNeighbors);
	}
	else
	{
		return Elite::Vector2{ 0, 0 };
	}
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
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
