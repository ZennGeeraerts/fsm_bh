#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock
{
public:
	Flock(
		Elite::Vector2* pMousePosition,
		Elite::Vector2* pOtherAgentPosition,
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const;
	const vector<SteeringAgent*>& GetNeighbors() const;

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;
	Elite::Vector2* m_pMousePosition;
	Elite::Vector2* m_pOtherAgentPosition;

	bool m_CanDebugRender{ false };
	
	// steering Behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;
	Seek* m_pSeek{ nullptr };
	Wander* m_pWander{ nullptr };
	Evade* m_pEvade{ nullptr };
	Cohesion* m_pCohesion{ nullptr };
	Seperation* m_pSeperation{ nullptr };
	Alignment* m_pAlignment{ nullptr };

	// Partitioning
	bool m_IsUsingPartitioning{ true };
	CellSpace* m_pCellSpace{ nullptr };
	float m_QueryRadius{ 15.f };
	int m_NrOfColls{ 10 };
	int m_NrOfRows{ 10 };
	std::vector<Elite::Vector2> m_OldPositions;

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};