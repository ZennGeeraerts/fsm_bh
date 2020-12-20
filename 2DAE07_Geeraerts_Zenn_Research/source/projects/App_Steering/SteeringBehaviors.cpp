//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"
#include <math.h>

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	return MoveToTarget(m_Target.Position, pAgent->GetPosition(), pAgent);
}

SteeringOutput Seek::MoveToTarget(const Elite::Vector2& targetPos, const Elite::Vector2& startPos, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = targetPos - startPos;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1, 0, 0.5f }, 0.40f);
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distanceToTarget{ Distance(pAgent->GetPosition(), m_Target.Position) };

	/*if (distanceToTarget > m_FleeRadius)
	{
		SteeringOutput steering;;
		steering.IsValid = false;
		return steering;
	}*/

	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };
	steering.LinearVelocity *= -1.f;

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const float slowRadius{ 15.f };;

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition();

	const float distance{ steering.LinearVelocity.Magnitude() };
	steering.LinearVelocity.Normalize();

	if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1, 0, 0.5f }, 0.40f);
	}

	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	Elite::Vector2 targetPos{ ((m_Target).Position - pAgent->GetPosition()) };
	float targetPosMagn{ targetPos.Magnitude() };

	if (targetPosMagn != 0)
	{
		float agentRot{ (int(pAgent->GetRotation() * 180 / float(M_PI)) % 360) * float(M_PI) / 180 };

		if (agentRot > float(M_PI))
		{
			agentRot = -(2 * float(M_PI) - agentRot);
		}
		else if (agentRot < -float(M_PI))
		{
			agentRot = 2 * float(M_PI) + agentRot;
		}

		Elite::Vector2 agentDir{ sin(agentRot) * targetPosMagn, -cos(agentRot) * targetPosMagn };

		if (!(((targetPos - agentDir).Magnitude() < 1.f) && ((targetPos - agentDir).Magnitude() > -1.f)))
		{
			steering.AngularVelocity = atan2f(-targetPos.x, targetPos.y);
			steering.AngularVelocity *= pAgent->GetMaxAngularSpeed();
		}
	}

	pAgent->SetAutoOrient(false);
	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float circleRadius{ 2.f };
	const float circleOffset{ 5.0f };
	const float agentOrientation{ pAgent->GetOrientation() };
	Elite::Vector2 agentDir{ sin(agentOrientation), -cos(agentOrientation) };
	Elite::Vector2 circleMid{ pAgent->GetPosition() + agentDir * circleOffset };

	const float rate{ 0.8f };
	float angle{ Elite::randomFloat(E_PI * 2) * rate };
	m_Target.Position = { circleMid.x + cos(angle) * circleRadius, circleMid.y + sin(angle) * circleRadius };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(circleMid, circleRadius, Elite::Color{ 1, 0, 0, 0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawSegment(pAgent->GetPosition(), m_Target.Position, Elite::Color{ 0, 0, 1, 0.5f });
		DEBUGRENDERER2D->DrawPoint(m_Target.Position, 4, Elite::Color{ 1, 1, 1, 0.5f });
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 distance{ m_Target.Position - pAgent->GetPosition() };
	float t{ distance.Magnitude() / pAgent->GetMaxLinearSpeed() };
	m_Target.Position = m_Target.Position + m_Target.LinearVelocity * t;
	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distance{ Distance(pAgent->GetPosition(), m_Target.Position) };

	if (distance > m_FleeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}
	Elite::Vector2 distanceVec{ m_Target.Position - pAgent->GetPosition() };
	float t{ distanceVec.Magnitude() / (m_Target.LinearVelocity.Magnitude() + pAgent->GetMaxLinearSpeed()) };
	m_Target.Position = m_Target.Position + m_Target.LinearVelocity * t;
	return Flee::CalculateSteering(deltaT, pAgent);
}