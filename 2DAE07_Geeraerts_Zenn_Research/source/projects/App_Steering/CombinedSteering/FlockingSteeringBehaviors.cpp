#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"
#include "../SteeringBehaviors.h"

//*********************
//SEPARATION (FLOCKING)
Seperation::Seperation(Flock* pFlock, float neighborRadius)
	: m_pFlock{ pFlock }
	, m_NeighborRadius{ neighborRadius }
{

}

SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	std::vector<SteeringAgent*> pNeighbors{ m_pFlock->GetNeighbors() };
	
	int nrOfNeighbors{ m_pFlock->GetNrOfNeighbors() };
	for (int i{}; i < nrOfNeighbors; ++i)
	{
		steering.LinearVelocity += (pNeighbors[i]->GetPosition() - pAgent->GetPosition()).GetNormalized() * (m_NeighborRadius * 2 - Elite::Distance(pNeighbors[i]->GetPosition(), pAgent->GetPosition()));
	}

	if (nrOfNeighbors > 0)
	{
		steering.LinearVelocity /= nrOfNeighbors;
		steering.LinearVelocity *= -1;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1, 0, 0.5f }, 0.40f);
	}

	return steering;
}

//*******************
//COHESION (FLOCKING)
Cohesion::Cohesion(Flock* pFlock)
	: m_pFlock{ pFlock }
{

}

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborPos() - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1, 0, 0.5f }, 0.40f);
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
Alignment::Alignment(Flock* pFlock)
	: m_pFlock{ pFlock }
{

}

SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Elite::GetNormalized(m_pFlock->GetAverageNeighborVelocity());

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1, 0, 0.5f }, 0.40f);
	}

	return steering;
}