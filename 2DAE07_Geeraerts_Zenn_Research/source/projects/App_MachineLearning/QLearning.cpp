#include "stdafx.h"
#include "QLearning.h"
#include <stdio.h>

QLearning::QLearning(int nrOfLocations, int startIndex, int endIndex)
	:m_pRewardMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pQMatrix(new FMatrix(nrOfLocations, nrOfLocations)),
	m_pTreasureMatrix{ new FMatrix{ nrOfLocations, nrOfLocations }},
	m_pKoboldMatrix{ new FMatrix{ nrOfLocations, nrOfLocations }},
	m_pEnvironmentMatrix{ new FMatrix{ nrOfLocations, nrOfLocations } },
	m_TreasureLocations{ 2 },
	m_KoboldLocations{ 4, 5, 6 },
	m_StartIndex(startIndex),
	m_EndIndex(endIndex),
	m_NrOfLocations(nrOfLocations),
	m_pIndexBuffer(new int[nrOfLocations])
{
	m_Locations.resize(nrOfLocations);
	m_pRewardMatrix->SetAll(-1.0f);
	m_pQMatrix->SetAll(0.0f);
	m_pTreasureMatrix->SetAll(0.0f);
	m_pKoboldMatrix->SetAll(0.0f);
	m_pEnvironmentMatrix->SetAll(0.0f);
	m_pRewardMatrix->Set(endIndex, endIndex, 100);

	for (size_t i{}; i < m_TreasureLocations.size(); ++i)
	{
		m_pTreasureMatrix->Set(m_TreasureLocations[i], m_TreasureLocations[i], 100.f);
	}

	for (size_t i{}; i < m_KoboldLocations.size(); ++i)
	{
		m_pKoboldMatrix->Set(m_KoboldLocations[i], m_KoboldLocations[i], 100.f);
	}
}

void QLearning::SetLocation(int index, Elite::Vector2 location)
{
	if (index < m_NrOfLocations) {
		m_Locations[index] = location;
	}
}

void QLearning::AddConnection(int from, int to)
{
	// ----------------------------------------------
	// connections are set in the m_pRewardMatrix !!!
	// ----------------------------------------------
	// set two cells corresponding to (from,to) and (to,from) to 0
	// unless the to is equal to the end index, then the (from,to) should be 100.
	// use the set method of the fmatrix class
	if (to == m_EndIndex)
	{
		m_pRewardMatrix->Set(from, to, 100.f);
		m_pRewardMatrix->Set(to, to, 100.f);
		m_pRewardMatrix->Set(to, from, 0.0f);
	}
	else
	{
		m_pRewardMatrix->Set(from, to, 0.0f);
		m_pRewardMatrix->Set(to, from, 0.0f);
	}
}

void QLearning::Render(float deltaTime)
{
	char buffer[10];
	Elite::Vector2 arrowPoints[3];
	for (int row = 0; row < m_NrOfLocations; ++row)
	{
		for (int column = 0; column < m_NrOfLocations; ++column)
		{
			if (m_pRewardMatrix->Get(row, column) >= 0.f) {

				Elite::Vector2 start = m_Locations[row];
				Elite::Vector2 end = m_Locations[column];
				float length = start.Distance(end);

				
				Elite::Vector2 dir = end - start;
				dir.Normalize();
				Elite::Vector2 perpDir(dir.y, -dir.x);
				

				Elite::Vector2 tStart = start + perpDir * 2;
				Elite::Vector2 tEnd = end + perpDir * 2;

				Elite::Vector2 mid = (tEnd + tStart) * .5 + 5 * dir;



				



				arrowPoints[0] = mid + dir * 5;
				arrowPoints[1] = mid + perpDir * 1.5f;
				arrowPoints[2] = mid - perpDir * 1.5f;

				float qValue = m_pQMatrix->Get(row, column);
				float max = m_pQMatrix->Max();
				float ip = qValue / max;
				float ipOneMinus = 1 - ip;
				Elite::Color c;
				c.r = m_NoQConnection.r * ipOneMinus + m_MaxQConnection.r * ip;
				c.g = m_NoQConnection.g * ipOneMinus + m_MaxQConnection.g * ip;
				c.b = m_NoQConnection.b * ipOneMinus + m_MaxQConnection.b * ip;
				DEBUGRENDERER2D->DrawSegment(tStart, tEnd, c);
				DEBUGRENDERER2D->DrawSolidPolygon(&arrowPoints[0], 3, c, 0.5);
				snprintf(buffer, 10, "%.0f", qValue);
				DEBUGRENDERER2D->DrawString(mid + perpDir*3, buffer);
			}
		}
	}

	int index = 0;
	

	for (Elite::Vector2 loc : m_Locations)
	{
		snprintf(buffer, 3, "%d", index);
		DEBUGRENDERER2D->DrawString(loc + Elite::Vector2(1.5f, 0), buffer);
		if (index == m_StartIndex)
		{
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_StartColor, 0.5f);
		}
		else if (index == m_EndIndex) {
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_EndColor, 0.5f);
		}
		else {
			DEBUGRENDERER2D->DrawSolidCircle(loc, 2.0f, Elite::Vector2(1, 0), m_NormalColor, 0.5f);
		}

		++index;
	}

}

int QLearning::SelectAction(int currentLocation)
{
	// Step 2 in the slides, select a to node via the reward matrix.
	// return this to-node as the result
	std::vector<int> connections{};
	for (int i{}; i < m_NrOfLocations; ++i)
	{
		float connectionReward{ m_pRewardMatrix->Get(currentLocation, i) };
		if (connectionReward >= 0.0f)
		{
			connections.push_back(i);
		}
	}

	// select random connection
	return connections[Elite::randomInt(connections.size())];
}

int QLearning::SelectActionWithEnv(int currentLocation)
{
	std::vector<int> connections{};
	for (int i{}; i < m_NrOfLocations; ++i)
	{
		float connectionReward{ m_pRewardMatrix->Get(currentLocation, i) };
		float connectionEnv{ m_pEnvironmentMatrix->Get(currentLocation, i) };
		if ((connectionReward >= 0.0f) && (connectionEnv >= 0.0f))
		{
			connections.push_back(i);
		}
	}

	if (connections.size() == 0)
	{
		return SelectAction(currentLocation);
	}

	/*return *(std::max_element(connections.begin(), connections.end(),
		[this, currentLocation](int i, int j)
		{
			float envResult1{ m_pEnvironmentMatrix->Get(currentLocation, i) };
			float envResult2{ m_pEnvironmentMatrix->Get(currentLocation, j) };
			return envResult1 < envResult2;
		}));*/
	return connections[Elite::randomInt(connections.size())];
}

float QLearning::Update(int currentLocation, int nextAction)
{
	for (size_t i{}; i < m_TreasureLocations.size(); ++i)
	{
		if (nextAction == m_TreasureLocations[i])
		{
			m_pTreasureMatrix->Add(currentLocation, nextAction, 1.0f);
		}
	}

	for (size_t i{}; i < m_KoboldLocations.size(); ++i)
	{
		if (nextAction == m_KoboldLocations[i])
		{
			m_pKoboldMatrix->Add(currentLocation, nextAction, 1.0f);
		}
	}

	m_pEnvironmentMatrix->Copy(*m_pTreasureMatrix);
	m_pEnvironmentMatrix->Subtract(*m_pKoboldMatrix);

	// step 3 
	// A : get the max q-value of the row nextAction in the Q matrix
	float max = m_pQMatrix->MaxOfRow(nextAction);

	// B gather the elements that are equal to this max in an index buffer.
	// can use m_pIndexBuffer if it suits you. Devise your own way if you don't like it.
	std::vector<int> indexBuffer{};
	for (int i{}; i < m_pQMatrix->GetNrOfColumns(); ++i)
	{
		float qToValue{ m_pQMatrix->Get(nextAction, i) };

		if (abs(max - qToValue) < 0.001f)
		{
			indexBuffer.push_back(i);
		}
	}

	// C pick a random index from the index buffer and implement the update formula
	// for the q matrix. (slide 14)
	int qTo{ indexBuffer[Elite::randomInt(indexBuffer.size())] };
	float qUpdate{ m_pRewardMatrix->Get(currentLocation, nextAction) + m_Gamma * m_pQMatrix->Get(nextAction, qTo) };
	m_pQMatrix->Set(currentLocation, nextAction, qUpdate);
	
	// calculate the score of the q-matrix and return it. (slide 15)
	if (m_pQMatrix->Sum() > 0.0f)
	{
		return 100 * m_pQMatrix->Sum() / m_pQMatrix->Max();
	}
	else
	{
		return 0.0f;
	}
}

void QLearning::Train() {
	if (m_CurrentIteration < m_NrOfIterations)
	{
		int currentLocation = Elite::randomInt(m_NrOfLocations);
		int nextLocation = SelectActionWithEnv(currentLocation);
		float score = Update(currentLocation, nextLocation);
		printf("Score %.2f\n", score);
		m_CurrentIteration++;
		
	}else if (m_CurrentIteration == m_NrOfIterations){
		m_pQMatrix->Print();
		//test from start point 0
		int location = m_StartIndex;

		printf("start at %d\t", location);
		
		// TODO : find the best path via the q-matrix.
		// uncomment the while loop when implementing, be careful for infinite loop.
		while (location != m_EndIndex)
		{
			// what is the maximum of the next action in the q-matrix
			std::vector<int> connections{};
			for (int i{}; i < m_pRewardMatrix->GetNrOfColumns(); ++i)
			{
				float connectionReward{ m_pRewardMatrix->Get(location, i) };
				if (connectionReward >= 0.0f)
				{
					connections.push_back(i);
				}
			}

			int to{ *(std::max_element(connections.begin(), connections.end(),
				[this, location] (int i, int j)
				{
					float qResult1{ m_pQMatrix->Get(location, i) };
					float qResult2{ m_pQMatrix->Get(location, j) };
					return qResult1 < qResult2;
				})
				)};

			location = to;

			printf("%d\t", location);
		}
		m_CurrentIteration++;
	}
}