//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "TheFlock.h"

//Destructor
App_Flocking::~App_Flocking()
{	
	SAFE_DELETE(m_pFlock);
	SAFE_DELETE(m_pOtherAgent);
	SAFE_DELETE(m_pWander);
}

//Functions
void App_Flocking::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 1.5f));

	m_pWander = new Wander{};
	m_pOtherAgent = new SteeringAgent{};
	m_pOtherAgent->SetBodyColor(Elite::Color{ 1, 0, 0 });
	m_pOtherAgent->SetSteeringBehavior(m_pWander);
	m_pOtherAgent->SetMaxLinearSpeed(15.f);
	m_pOtherAgent->SetAutoOrient(true);
	m_pOtherAgent->SetMass(1.f);
	m_pOtherAgent->SetPosition({ 0.0f, 0.0f });

	m_pFlock = new Flock{ &m_MouseTarget.Position, &m_pOtherAgent->GetPosition(), 2000, m_TrimWorldSize, m_pOtherAgent, true };
}

void App_Flocking::Update(float deltaTime)
{
	m_pOtherAgent->Update(deltaTime);
	m_pOtherAgent->TrimToWorld(m_TrimWorldSize);
	m_pFlock->Update(deltaTime);

	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}
}

void App_Flocking::Render(float deltaTime) const
{
	m_pOtherAgent->Render(deltaTime);
	m_pFlock->Render(deltaTime);

	std::vector<Elite::Vector2> points
	{
		{ -m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, -m_TrimWorldSize },
		{ -m_TrimWorldSize, -m_TrimWorldSize }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
