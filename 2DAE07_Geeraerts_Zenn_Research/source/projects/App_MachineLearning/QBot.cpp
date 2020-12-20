#include "stdafx.h"
#include "QBot.h"

// for setting the precision in cout for floating points.
#include <iomanip>


void QBot::Update(vector<Food*>& foodList, float deltaTime)
{
	m_Age += deltaTime;
	currentIndex = (currentIndex + 1) % m_MemorySize;
	if (!m_Alive) {
		return;
	}
	m_Visible.clear();
	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	float angleStep = m_FOV / (m_NrOfInputs);
	m_StateMatrixMemoryArr[currentIndex].SetAll(0.0);
	
	for (Food* food : foodList) {
		if (food->IsEaten()) {
			continue;
		}
		Vector2 foodLoc = food->GetLocation();
		Vector2 foodVector = foodLoc - (m_Location - dir * 10);
		float dist = (foodLoc-m_Location).Magnitude();
		if (dist > m_MaxDistance) {
			continue;
		}
		foodVector *= 1 / dist;

		float angle = AngleBetween(dir, foodVector);
		if (angle > -m_FOV / 2 && angle < m_FOV / 2) {
			m_Visible.push_back(food);

			int index = (int)((angle + m_FOV / 2) / angleStep);
			float invDist = CalculateInverseDistance(dist);
			float currentDist = m_StateMatrixMemoryArr[currentIndex].Get(0, index);
			if (invDist > currentDist) {
				m_StateMatrixMemoryArr[currentIndex].Set(0, index, invDist);
			}
		}

		if (dist < 2.0f) {
			food->Eat();
			m_CameCloseCounter = 50;
			m_FoodEaten++;
			m_Health += 30.0f;
			Reinforcement(m_PositiveQ,m_MemorySize);
		}
	}
	

	if (m_CameCloseCounter > 0) {
		m_CameCloseCounter--;
	}

	for (Food* food : foodList)
	{
		if (!food->IsEaten())
		{
			Vector2 foodLoc = food->GetLocation();
			float dist = (foodLoc - m_Location).Magnitude();
			if ((dist < 10.f) && (std::find(m_Visible.begin(), m_Visible.end(), food) != m_Visible.end()))
			{
				Reinforcement(m_NegativeQClose, m_MemorySize);
				continue;
			}
		}
	}

	m_StateMatrixMemoryArr[currentIndex].Set(0, m_NrOfInputs, 1); //bias
	m_StateMatrixMemoryArr[currentIndex].MatrixMultiply(m_BotBrain, m_ActionMatrixMemoryArr[currentIndex]);
	m_ActionMatrixMemoryArr[currentIndex].Sigmoid();

	int r, c;
	float max = m_ActionMatrixMemoryArr[currentIndex].Max(r, c);

	float dAngle = m_SAngle.Get(0, c);
	m_Angle += dAngle *deltaTime;

	Elite::Vector2 newDir(cos(m_Angle), sin(m_Angle));
	m_Location += newDir * m_Speed*deltaTime;

	m_Health -= 0.1f;
	if (m_Health < 0) {
		// update the bot brain, something went wrong.
		Reinforcement(m_NegativeQ,m_MemorySize);
		m_Health = 100.0f;
		m_Location = m_StartLocation;
		
		cout << "Died after "<< std::setprecision(4) << m_Age << " seconds." << endl;
		m_Age = 0;
	}
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(m_Location);
}

void QBot::Render(float deltaTime) {
	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	Elite::Vector2 leftVision(cos(m_Angle + m_FOV / 2), sin(m_Angle + m_FOV / 2));
	Elite::Vector2 rightVision(cos(m_Angle - m_FOV / 2), sin(m_Angle - m_FOV / 2));

	Elite::Vector2 perpDir(-dir.y, dir.x);

	Color c = m_DeadColor;
	if (m_Alive) {
		c = m_AliveColor;
	}

	DEBUGRENDERER2D->DrawSolidCircle(m_Location, 2, dir, c);
	if (m_Alive) {
		DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * leftVision, c);
		DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * rightVision, c);
	}
	DEBUGRENDERER2D->DrawString(m_Location, to_string((int)m_Health).c_str());

	if (m_Alive) {
		for (Food* f : m_Visible) {
			Vector2 loc = f->GetLocation();
			DEBUGRENDERER2D->DrawCircle(loc, 2, c, 0.5f);
		}
	}

	// draw the vision
	for (int i = 0; i < m_NrOfInputs; ++i)
	{

		if (m_StateMatrixMemoryArr[currentIndex].Get(0, i) > 0.0f) {
			DEBUGRENDERER2D->DrawSolidCircle(m_Location - 2.5 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_AliveColor);
		}
		else {
			DEBUGRENDERER2D->DrawSolidCircle(m_Location - 3.0 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_DeadColor);
		}
	}

	char age[10];
	snprintf(age, 10, "%.1f seconds", m_Age);
	DEBUGRENDERER2D->DrawString(m_Location + m_MaxDistance * dir, age);
}

void QBot::Reinforcement(float factor,int memory)
{
	// go back in time, and reinforce (or inhibit) the weights that led to the right/wrong decision.
	m_DeltaBotBrain.SetAll(0);
	int minimum{ min(memory, m_MemorySize) };

	for (int mi{}; mi < minimum; ++mi)
	{
		float timeFactor{ 1.f / (1.f + (mi * mi)) };
		int actualIndex{ currentIndex - mi };
		
		if (actualIndex < 0)
		{
			actualIndex += m_MemorySize;
		}

		int cMax{};
		int rMax{};
		m_ActionMatrixMemoryArr[actualIndex].Max(rMax, cMax);

		for (int i{}; i < m_NrOfInputs; ++i)
		{
			float stateMatrixValue{ m_StateMatrixMemoryArr[actualIndex].Get(0, i) };

			if (stateMatrixValue > 0)
			{
				m_DeltaBotBrain.Add(i, cMax, factor * timeFactor * stateMatrixValue);
				// Select random column other then cMax
				int rcMax{};
				do
				{
					int nrOfColls{ m_DeltaBotBrain.GetNrOfColumns() };
					rcMax = Elite::randomInt(nrOfColls);
				} while (rcMax == cMax);

				m_DeltaBotBrain.Add(i, rcMax, factor * -timeFactor * stateMatrixValue);
			}
		}

		m_DeltaBotBrain.ScalarMultiply(1.f / m_MemorySize);
		m_BotBrain.Add(m_DeltaBotBrain);
	}
}