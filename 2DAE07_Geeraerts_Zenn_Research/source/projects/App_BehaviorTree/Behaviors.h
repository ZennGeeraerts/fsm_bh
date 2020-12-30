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

bool IsCloseToEnemy(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	std::vector<AgarioAgent*>* agentsVec{ nullptr };

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", agentsVec);

	if (!pAgent || !agentsVec)
		return false;

	// find close enemies
	const float closeToEnemyRange{ 20.f };
	std::vector<AgarioAgent*> closeAgents{};
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
			closeAgents.push_back((*agentsVec)[i]);
		}
	}

	// Get the closest enemy of the closeAgents
	auto it = std::min_element(closeAgents.begin(), closeAgents.end(), [pAgent](AgarioAgent* pAgarioAgent1, AgarioAgent* pAgarioAgent2)
		{
			float distanceEnemy1Sqr{ (pAgarioAgent1->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
			float distanceEnemy2Sqr{ (pAgarioAgent2->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
			return distanceEnemy1Sqr < distanceEnemy2Sqr;
		});

	if ((it != closeAgents.end()) && (!(*it)->CanBeDestroyed()))
	{
		pBlackboard->ChangeData("ClosestEnemy", (*it));
		return true;
	}

	return false;
}

bool IsClosestEnemyBigger(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	AgarioAgent* pClosestEnemy{ nullptr };

	auto dataAvailable{ pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("ClosestEnemy", pClosestEnemy) };
	if (!dataAvailable)
	{
		return false;
	}

	if (pAgent->GetRadius() < pClosestEnemy->GetRadius())
	{
		pBlackboard->ChangeData("Target", pClosestEnemy->GetPosition());
		return true;
	}

	return false;
}

bool IsClosestEnemySmaller(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	AgarioAgent* pClosestEnemy{ nullptr };

	auto dataAvailable{ pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("ClosestEnemy", pClosestEnemy) };
	if (!dataAvailable)
	{
		return false;
	}

	const float radiusOffset{ 2.0f };
	if (pAgent->GetRadius() > (pClosestEnemy->GetRadius() + radiusOffset))
	{
		pBlackboard->ChangeData("Target", pClosestEnemy->GetPosition());
		return true;
	}

	return false;
}

bool IsCloseToBorder(Elite::Blackboard* pBlackboard)
{
	float worldSize{};
	AgarioAgent* pAgent{ nullptr };
	auto dataAvailable = pBlackboard->GetData("WorldSize", worldSize) && pBlackboard->GetData("Agent", pAgent);

	if (!dataAvailable)
	{
		return false;
	}

	const float maxDistanceToBorder{ 10.f };
	const Elite::Vector2 agentPosition{ pAgent->GetPosition() };

	if (agentPosition.x < (-worldSize + maxDistanceToBorder))
	{
		pBlackboard->ChangeData("Target", Elite::Vector2{ -worldSize, agentPosition.y });
		return true;
	}
	else if (agentPosition.x > (worldSize - maxDistanceToBorder))
	{
		pBlackboard->ChangeData("Target", Elite::Vector2{ worldSize, agentPosition.y });
		return true;
	}
	else if (agentPosition.y < (-worldSize + maxDistanceToBorder))
	{
		pBlackboard->ChangeData("Target", Elite::Vector2{ agentPosition.x, -worldSize });
		return true;
	}
	else if (agentPosition.y > (worldSize - maxDistanceToBorder))
	{
		pBlackboard->ChangeData("Target", Elite::Vector2{ agentPosition.x, worldSize });
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

BehaviorState ChangeToPursuit(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 pursuitTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Target", pursuitTarget);

	if (!dataAvailable)
	{
		return Failure;
	}

	if (!pAgent)
		return Failure;

	pAgent->SetToPursuit(pursuitTarget);

	return Success;
}

BehaviorState RunFSM(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent{ nullptr };
	FiniteStateMachine* pFSM{ nullptr };

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FSM", pFSM);

	if (!dataAvailable)
	{
		return Failure;
	}

	if (!pAgent)
		return Failure;

	std::cout << "Run FSM\n";
	pAgent->SetDecisionMaking(pFSM);

	return Success;
}
#endif