/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
bool IsCloseToFood(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* foodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", foodVec);

	if (!pAgent || !foodVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float closeToFoodRange{ 20.f };
	auto foodIt{ std::find_if((*foodVec).begin(), (*foodVec).end(), [&pAgent, &closeToFoodRange](AgarioFood* f)
		{
			return DistanceSquared(pAgent->GetPosition(), f->GetPosition()) < (closeToFoodRange * closeToFoodRange);
		}
	) };

	if (foodIt != (*foodVec).end())
	{
		pBlackboard->ChangeData("Target", (*foodIt)->GetPosition());
		return true;
	}

	return false;
}

bool IsCloseToBiggerEnemy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	std::vector<AgarioAgent*>* agentsVec{ nullptr };

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", agentsVec);

	if (!pAgent || !agentsVec)
		return false;

	// find close enemies
	const float closeToEnemyRange{ 20.f };
	std::vector<AgarioAgent*> closeByBiggerAgents{};
	for (size_t i{}; i < (*agentsVec).size(); ++i)
	{
		if ((*agentsVec)[i]->CanBeDestroyed())
		{
			continue;
		}

		float distancePlayerEnemy{ ((*agentsVec)[i]->GetPosition() - pAgent->GetPosition()).Magnitude() };
		distancePlayerEnemy -= pAgent->GetRadius();
		distancePlayerEnemy -= (*agentsVec)[i]->GetRadius();

		if (distancePlayerEnemy < closeToEnemyRange)
		{
			if ((*agentsVec)[i]->GetRadius() > pAgent->GetRadius())
			{
				closeByBiggerAgents.push_back((*agentsVec)[i]);
			}
		}
	}

	auto it = std::min_element(closeByBiggerAgents.begin(), closeByBiggerAgents.end(), [pAgent](AgarioAgent* pAgarioAgent1, AgarioAgent* pAgarioAgent2)
		{
			float distanceFood1Sqr{ (pAgarioAgent1->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
			float distanceFood2Sqr{ (pAgarioAgent2->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
			return distanceFood1Sqr < distanceFood2Sqr;
		});

	if ((it != closeByBiggerAgents.end()) && (!(*it)->CanBeDestroyed()))
	{
		// set flee target to it
		pBlackboard->ChangeData("Target", (*it)->GetPosition());
		return true;
	}

	return false;
}

BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return Failure;
	}

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();

	return Success;
}

BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable)
	{
		return Failure;
	}

	if (!pAgent)
		return Failure;
	
	//TODO: Implement Change to seek (Target)
	pAgent->SetToSeek(seekTarget);

	return Success;
}

BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 fleeTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", fleeTarget);

	if (!dataAvailable)
	{
		return Failure;
	}

	if (!pAgent)
		return Failure;

	//TODO: Implement Change to seek (Target)
	pAgent->SetToFlee(fleeTarget);

	return Success;
}

#endif