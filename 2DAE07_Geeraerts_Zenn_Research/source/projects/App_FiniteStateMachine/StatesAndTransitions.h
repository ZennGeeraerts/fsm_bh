/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

// Agario agent states
class WanderState : public Elite::FSMState
{
public:
	WanderState()
		: FSMState()
	{

	}

	virtual void OnEnter(Blackboard* pB) override
	{
		// agent opvragen uit blackboard
		AgarioAgent* pAgent{ nullptr };
		bool dataAvailable{ pB->GetData("Agent", pAgent) };
		if (!dataAvailable || !pAgent)
		{
			return;
		}

		// agent wandering steering behaviour actief zetten
		pAgent->SetToWander();
	}
};

class SeekFoodState : public Elite::FSMState
{
public:
	SeekFoodState()
		: FSMState()
	{

	}

	virtual void OnEnter(Blackboard* pB) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool dataAvailable{ pB->GetData("Agent", pAgent) };
		if (!dataAvailable || !pAgent)
		{
			return;
		}

		AgarioFood* target{ nullptr };
		dataAvailable = pB->GetData("FoodTarget", target);
		if (!dataAvailable)
		{
			return;
		}

		if (!target->CanBeDestroyed())
		{
			pAgent->SetToSeek(target->GetPosition());
		}
	}
};

class EvadeBiggerEnemyState : public FSMState
{
public:
	EvadeBiggerEnemyState()
		: FSMState()
	{

	}

	virtual void OnEnter(Blackboard* pB) override
	{
		AgarioAgent* pAgent{ nullptr };
		bool dataAvailable{ pB->GetData("Agent", pAgent) };
		if (!dataAvailable || !pAgent)
		{
			return;
		}

		AgarioAgent* target{ nullptr };
		dataAvailable = pB->GetData("AgentTarget", target);
		if (!dataAvailable)
		{
			return;
		}

		if (!target->CanBeDestroyed())
		{
			pAgent->SetToFlee(target->GetPosition());
		}
	}
};

// Agario agent transitions
class CloseToFood : public Elite::FSMTransition
{
public:
	CloseToFood()
		: FSMTransition()
	{

	}

	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		std::vector<AgarioFood*>* pFoodVec{ nullptr };
		pBlackboard->GetData("FoodVec", pFoodVec);

		AgarioAgent* pAgent{ nullptr };
		pBlackboard->GetData("Agent", pAgent);
		// is food close by?
		// return true
		// else return false

		std::vector<AgarioFood*> closeByFood{};
		for (size_t i{}; i < (*pFoodVec).size(); ++i)
		{
			if ((*pFoodVec)[i]->CanBeDestroyed())
			{
				continue;
			}

			float distanceAgentFood{ ((*pFoodVec)[i]->GetPosition() - pAgent->GetPosition()).Magnitude() };

			if ((distanceAgentFood - pAgent->GetRadius() < m_CloseByRange))
			{
				closeByFood.push_back((*pFoodVec)[i]);
			}
		}

		auto it = std::min_element(closeByFood.begin(), closeByFood.end(), [pAgent](AgarioFood* pAgarioFood1, AgarioFood* pAgarioFood2)
			{
				float distanceFood1Sqr{ (pAgarioFood1->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
				float distanceFood2Sqr{ (pAgarioFood2->GetPosition() - pAgent->GetPosition()).MagnitudeSquared() };
				return distanceFood1Sqr < distanceFood2Sqr;
			});

		if ((it != closeByFood.end()) && (!(*it)->CanBeDestroyed()))
		{
			// set seek target to it
			pBlackboard->ChangeData("FoodTarget", *it);
			// return true
			return true;
		}
		else
		{
			return false;
		}
	}

protected:
	const float m_CloseByRange{ 15.f };
};

class AteFood : public FSMTransition
{
public:
	AteFood()
		: FSMTransition()
	{

	}

	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		pBlackboard->GetData("Agent", pAgent);

		AgarioFood* pFood{ nullptr };
		pBlackboard->GetData("FoodTarget", pFood);
		// is food close by?
		// return true
		// else return false

		if (pFood->CanBeDestroyed())
		{
			return true;
		}

		float distanceAgentFood{ (pFood->GetPosition() - pAgent->GetPosition()).Magnitude() };

		if ((distanceAgentFood - pAgent->GetRadius()) < 0.01f)
		{
			return true;
		}

		return false;
	}
};

class CloseToBiggerEnemy : public FSMTransition
{
public:
	CloseToBiggerEnemy()
		: FSMTransition()
	{

	}

	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		std::vector<AgarioAgent*>* pAgentVec{ nullptr };
		pBlackboard->GetData("AgentVec", pAgentVec);

		AgarioAgent* pAgent{ nullptr };
		pBlackboard->GetData("Agent", pAgent);
		// is food close by?
		// return true
		// else return false

		std::vector<AgarioAgent*> closeByBiggerAgents{};
		for (size_t i{}; i < (*pAgentVec).size(); ++i)
		{
			if ((*pAgentVec)[i]->CanBeDestroyed())
			{
				continue;
			}

			float distancePlayerEnemy{ ((*pAgentVec)[i]->GetPosition() - pAgent->GetPosition()).Magnitude() };
			distancePlayerEnemy -= pAgent->GetRadius();
			distancePlayerEnemy -= (*pAgentVec)[i]->GetRadius();

			if (distancePlayerEnemy < m_CloseByRange)
			{
				if ((*pAgentVec)[i]->GetRadius() > pAgent->GetRadius())
				{
					closeByBiggerAgents.push_back((*pAgentVec)[i]);
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
			// set seek target to it
			pBlackboard->ChangeData("AgentTarget", *it);
			// return true
			return true;
		}
		else
		{
			return false;
		}
	}
protected:
	const float m_CloseByRange{ 20.f };
};

class EvadedEnemy : public FSMTransition

{
public:
	EvadedEnemy()
		: FSMTransition()
	{

	}

	virtual bool ToTransition(Blackboard* pBlackboard) const override
	{
		AgarioAgent* pAgent{ nullptr };
		pBlackboard->GetData("Agent", pAgent);

		AgarioAgent* pEnemy{ nullptr };
		pBlackboard->GetData("AgentTarget", pEnemy);
		// is food close by?
		// return true
		// else return false

		
		if (pEnemy->CanBeDestroyed())
		{
			return true;
		}

		float distancePlayerEnemy{ (pEnemy->GetPosition() - pAgent->GetPosition()).Magnitude() };
		distancePlayerEnemy -= pAgent->GetRadius();
		distancePlayerEnemy -= pEnemy->GetRadius();

		if (distancePlayerEnemy > m_CloseByRange)
		{
			return true;
		}

		return false;
	}

private:
	const float m_CloseByRange{ 20.f };
};
#endif