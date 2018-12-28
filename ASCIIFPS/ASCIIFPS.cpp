//Basic Console FPS
#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

int windowWidth = 120;
int windowHeight = 40;

float playerX = 0.0f;
float playerY = 0.0f;
float playerLookAngle = 0.0f;
float turnRate = 0.8f;
float moveSpeed = 5.0f;
float FOV = 3.14f/4.0f;
int mapHeight = 20;
int mapWidth = 20;
float maxRayDistance = 20.0f;

int main()
{
	wchar_t *window = new wchar_t[windowWidth*windowHeight];
	for (int i = 0; i < windowWidth*windowHeight; i++)
		window[i] = L' ';
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;
	
	wstring map;
	map += L"####################";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#...#######........#";
	map += L"#.........#.......##";
	map += L"#.........#.......##";
	map += L"#.........#.......##";
	map += L"#.........#.......##";
	map += L"#.........#.......##";
	map += L"#.................##";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"#..................#";
	map += L"####################";

	playerX = 8.0f;
	playerY = 8.0f;

	bool isGameOver = false;

	auto timePoint1 = chrono::system_clock::now();
	auto timePoint2 = chrono::system_clock::now();

	while (!isGameOver)
	{
		//Calculate tick
		timePoint2 = chrono::system_clock::now();
		chrono::duration<float> TelapsedTime = timePoint2 - timePoint1;
		timePoint1 = timePoint2;
		float elapsedTime = TelapsedTime.count();

		//Handle Input
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			playerLookAngle -= turnRate * elapsedTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			playerLookAngle += turnRate * elapsedTime;
		
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			playerX += sinf(playerLookAngle) * moveSpeed * elapsedTime;
			playerY += cosf(playerLookAngle) * moveSpeed * elapsedTime;
			if (map[(int)playerY * mapWidth + (int)playerX] == '#')
			{
				playerX -= sinf(playerLookAngle) * moveSpeed * elapsedTime;
				playerY -= cosf(playerLookAngle) * moveSpeed * elapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			playerX -= sinf(playerLookAngle) * moveSpeed * elapsedTime;
			playerY -= cosf(playerLookAngle) * moveSpeed * elapsedTime;
			if (map[(int)playerY * mapWidth + (int)playerX] == '#')
			{
				playerX += sinf(playerLookAngle) * moveSpeed * elapsedTime;
				playerY += cosf(playerLookAngle) * moveSpeed * elapsedTime;
			}
		}
		
		for (int x = 0; x < windowWidth; x++)
		{
			float rayAngle = (playerLookAngle - FOV / 2.0f) + ((float)x / (float)windowWidth) * FOV;
			float distanceToCollision = 0.0f;
			bool hasCollided = false;
			bool boundary = false;

			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);
			
			while (!hasCollided && distanceToCollision < maxRayDistance)
			{
				distanceToCollision += 0.1f;
				int hitX = (int)(playerX + eyeX * distanceToCollision);
				int hitY = (int)(playerY + eyeY * distanceToCollision);

				if (hitX < 0 || hitX >= mapWidth || hitY < 0 || hitY >= mapHeight)
				{
					hasCollided = true;
					distanceToCollision = maxRayDistance;
				}
				else
				{
					if (map[hitY * mapWidth + hitX] == '#')
					{
						hasCollided = true;
						vector<pair<float, float>> p; // distance, dot-product
						for (int i = 0; i < 2; i++)
						{
							for (int j = 0; j < 2; j++)
							{
								float vecY = (float)hitY + j - playerX;
								float vecX = (float)hitX + i - playerX;
								float distance = sqrt(vecX*vecX + vecY * vecY);
								float dot = (eyeX * vecX / distance) + (eyeY * vecY / distance);
								p.push_back(make_pair(distance, dot));
							}
						}
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> & right) { return left.first < right.first; });

						float angleBound = 0.01;
						if (acos(p.at(0).second) < angleBound) boundary = true;
						if (acos(p.at(1).second) < angleBound) boundary = true;
						if (acos(p.at(2).second) < angleBound) boundary = true;
					}
				}
			}
			int ceiling = (float)(windowHeight / 2.0) - windowHeight / ((float)distanceToCollision); // calculate relative height of ceiling to create perspective of depth
			int floor = windowHeight - ceiling;

			short shade = ' ';
			//using some unicode shaded block characters
			if (distanceToCollision <= maxRayDistance / 4.0f) shade = 0x2588; // close
			else if (distanceToCollision <= maxRayDistance / 3.0f) shade = 0x2593;
			else if (distanceToCollision <= maxRayDistance / 2.0f) shade = 0x2592;
			else if (distanceToCollision <= maxRayDistance) shade = 0x2591;
			else shade = ' ';												// far

			if (boundary)  shade = ' ';

			for (int y = 0; y < windowHeight; y++)
			{
				if (y <= ceiling)
				{
					window[y*windowWidth + x] = ' ';
				}
				else if(y>ceiling && y<=floor)
				{
					window[y*windowWidth + x] = shade;
				}
				else
				{
					float floorVal = 1.0f - (((float)y - windowHeight / 2.0f) / ((float)windowHeight / 2.0f));
					if (floorVal < 0.25) shade = '#';
					else if (floorVal < 0.5) shade = 'X';
					else if (floorVal < 0.75) shade = '.';
					else if (floorVal < 0.9) shade = '-';
					else  shade = ' ';
					window[y*windowWidth + x] = shade;
				}
			}
			
		}
		if (timePoint1 == timePoint2) elapsedTime = 1;
		swprintf_s(window, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS = %3.2f", playerX, playerY, playerLookAngle, 1.0f / elapsedTime);
		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
			{
				window[(y + 1) * windowWidth + x] = map[y * mapWidth + x];
			}
		}
		window[((int)playerY + 1) * windowWidth + (int)playerX] = 'P';
		window[windowWidth * windowHeight - 1] = '\0';
		WriteConsoleOutputCharacter(console, window, windowHeight * windowWidth, { 0,0 }, &bytesWritten);
	}
	
	return 0;
}

