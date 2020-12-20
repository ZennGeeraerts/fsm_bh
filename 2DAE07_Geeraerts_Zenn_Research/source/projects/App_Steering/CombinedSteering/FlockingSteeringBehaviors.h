#pragma once
#include "../SteeringBehaviors.h"
class Flock;

//SEPARATION - FLOCKING
//*********************
class Seperation : public ISteeringBehavior
{
public:
	Seperation(Flock* pFlock, float neighborRadius);
	virtual ~Seperation() = default;

	//Cohesion behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock{ nullptr };
	float m_NeighborRadius;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public ISteeringBehavior
{
public:
	Cohesion(Flock* pFlock);
	virtual ~Cohesion() = default;

	//Cohesion behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock{ nullptr };
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public ISteeringBehavior
{
public:
	Alignment(Flock* pFlock);
	virtual ~Alignment() = default;

	//Cohesion behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pFlock{ nullptr };
};