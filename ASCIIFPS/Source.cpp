#include <iostream>
#include "ConsoleGameEngine.h"
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;
std::atomic<bool> ConsoleGameEngine::atomActive(false);
std::condition_variable ConsoleGameEngine::gameFinished;
std::mutex ConsoleGameEngine::muxGame;


class ConsoleFPS : public ConsoleGameEngine
{
private:
	Vector2 playerPos = Vector2(0.0f, 0.0f);
	float playerLookAngle = -3.14159/2.0f;
	float turnRate = 0.8f;
	float moveSpeed = 5.0f;
	float FOV = 3.14159f / 4.0f;
	int mapHeight = 32;
	int mapWidth = 32;
	float maxRayDistance = 20.0f;
	float * depthBuffer;
	
	wstring map;
	Sprite * wallSprite;
	Sprite * fireballSprite;
	Sprite * statueSprite;

	struct renderObjects
	{
		float x;
		float y;
		float velX;
		float velY;
		bool toDestroy;
		Sprite * sprite;
	};
	list<renderObjects> rObjects;

public:

	ConsoleFPS()
	{
		appName = L'Console FPS';
	}

	bool virtual Start()
	{
		map += L"#########.......#########.......";
		map += L"#...............#...............";
		map += L"#.......#########.......########";
		map += L"#..............##..............#";
		map += L"#......##......##......##......#";
		map += L"#......##..............##......#";
		map += L"#..............##..............#";
		map += L"###............####............#";
		map += L"##.............###.............#";
		map += L"#............####............###";
		map += L"#..............................#";
		map += L"#..............##..............#";
		map += L"#..............##..............#";
		map += L"#...........#####...........####";
		map += L"#..............................#";
		map += L"###..####....########....#######";
		map += L"####.####.......######..........";
		map += L"#...............#...............";
		map += L"#.......#########.......##..####";
		map += L"#..............##..............#";
		map += L"#......##......##.......#......#";
		map += L"#......##......##......##......#";
		map += L"#..............##..............#";
		map += L"###............####............#";
		map += L"##.............###.............#";
		map += L"#............####............###";
		map += L"#..............................#";
		map += L"#..............................#";
		map += L"#..............##..............#";
		map += L"#...........##..............####";
		map += L"#..............##..............#";
		map += L"################################";

		wallSprite = new Sprite(L"wall.spr");
		statueSprite = new Sprite(L"statue.spr");
		fireballSprite = new Sprite(L"fireball.spr");
		depthBuffer = new float[windowWidth];
		//create some "statues"
		rObjects = {
		{ 9.5f,10.5f,0.0f,0.0f,false, statueSprite },
		{ 7.5f,7.5f,0.0f,0.0f,false,statueSprite },
		{ 13.5f,4.5f,0.0f,0.0f,false,statueSprite },
		};

		playerPos = Vector2(8.0f,8.0f);


		return true;
	}

	bool virtual Update(float elapsedTime)
	{
		getInput(elapsedTime);
		UpdateScreen();
		return true;
	}

	void CreateSprites()
	{

	}

	void getInput(float elapsedTime)
	{
		if (keys['A'].onHeld)
			playerLookAngle -= turnRate * elapsedTime;
		if (keys['D'].onHeld)
			playerLookAngle += turnRate * elapsedTime;

		if (keys['W'].onHeld)
		{
			playerPos.x += sinf(playerLookAngle) * moveSpeed * elapsedTime;
			playerPos.y += cosf(playerLookAngle) * moveSpeed * elapsedTime;
			if (map[(int)playerPos.y * mapWidth + (int)playerPos.x] == '#')
			{
				playerPos.x -= sinf(playerLookAngle) * moveSpeed * elapsedTime;
				playerPos.y -= cosf(playerLookAngle) * moveSpeed * elapsedTime;
			}
		}
		if (keys['S'].onHeld)
		{
			playerPos.x -= sinf(playerLookAngle) * moveSpeed * elapsedTime;
			playerPos.y -= cosf(playerLookAngle) * moveSpeed * elapsedTime;
			if (map[(int)playerPos.y * mapWidth + (int)playerPos.x] == '#')
			{
				playerPos.x += sinf(playerLookAngle) * moveSpeed * elapsedTime;
				playerPos.y += cosf(playerLookAngle) * moveSpeed * elapsedTime;
			}
		}
		if (keys['E'].onHeld)
		{
			playerPos.x += sinf(playerLookAngle) * moveSpeed * elapsedTime;
			playerPos.y -= cosf(playerLookAngle) * moveSpeed * elapsedTime;
			if (map[(int)playerPos.y * mapWidth + (int)playerPos.x] == '#')
			{
				playerPos.x -= sinf(playerLookAngle) * moveSpeed * elapsedTime;
				playerPos.y -= cosf(playerLookAngle) * moveSpeed * elapsedTime;
			}
		}
		if (keys['Q'].onHeld)
		{
			playerPos.x -= sinf(playerLookAngle) * moveSpeed * elapsedTime;
			playerPos.y += cosf(playerLookAngle) * moveSpeed * elapsedTime;
			if (map[(int)playerPos.y * mapWidth + (int)playerPos.x] == '#')
			{
				playerPos.x += sinf(playerLookAngle) * moveSpeed * elapsedTime;
				playerPos.y += cosf(playerLookAngle) * moveSpeed * elapsedTime;
			}
		}
		if (keys[VK_SPACE].onReleased)
		{
			renderObjects bullet;
			bullet.x = playerPos.x;
			bullet.y = playerPos.y;

			bullet.velX = sinf(playerLookAngle) * 1.0f;
			bullet.velY = cosf(playerLookAngle) * 1.0f;

			bullet.sprite = fireballSprite;
			bullet.toDestroy = false;
			rObjects.push_back(bullet);
		}
	}

	void UpdateScreen()
	{
		for (int x = 0; x < windowWidth; x++)
		{
			float rayAngle = (playerLookAngle - FOV / 2.0f) + ((float)x / (float)windowWidth) * FOV;
			float distanceToCollision = 0.0f;
			bool hasCollided = false;
			bool boundary = false;

			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);
			float sampleX = 0.0f; // how far across the texture are we
			while (!hasCollided && distanceToCollision < maxRayDistance)
			{
				distanceToCollision += 0.05f;
				int hitX = (int)(playerPos.x + eyeX * distanceToCollision);
				int hitY = (int)(playerPos.y + eyeY * distanceToCollision);

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

						//find mid point of hit block
						float midPointX = (float)hitX + 0.5f;
						float midPointY = (float)hitY + 0.5f;
						
						float testPointX = playerPos.x + eyeX * distanceToCollision;
						float testPointY = playerPos.y + eyeY * distanceToCollision;

						float testAngle = atan2f((testPointY - midPointY), (testPointX - midPointX)); //find the quadrant of the block the collision happened to
						

						if (testAngle >= -3.14159 * 0.25f && testAngle < 3.14159 * 0.25f)
							sampleX = testPointY - (float)hitY;

						if (testAngle >= 3.14159 * 0.25f && testAngle < 3.14159 * 0.75f)
							sampleX = testPointX - (float)hitX;

						if (testAngle < -3.14159 * 0.25f && testAngle >= -3.14159 * 0.75f)
							sampleX = testPointX - (float)hitX;

						if (testAngle >= 3.14159 * 0.75f && testAngle < -3.14159 * 0.75f)
							sampleX = testPointY - (float)hitY;

					}
				}
			}
			int ceiling = (float)(windowHeight / 2.0) - windowHeight / ((float)distanceToCollision); // calculate relative height of ceiling to create perspective of depth
			int floor = windowHeight - ceiling;

			depthBuffer[x] = distanceToCollision;
			for (int y = 0; y < windowHeight; y++)
			{
				if (y <= ceiling)
				{
					Draw(x, y, L' ');
				}
				else if (y > ceiling && y <= floor)
				{
					if (distanceToCollision < maxRayDistance)
					{
						float sampleY = ((float)y - (float)ceiling) / ((float)floor - (float)ceiling);
						Draw(x, y, wallSprite->SampleGlyph(sampleX, sampleY), wallSprite->SampleColour(sampleX, sampleY));
					}
					else
					{
						Draw(x, y, PIXEL_SOLID, FG_BLACK);
					}
					
				}
				else
				{

					Draw(x,y,PIXEL_SOLID, FG_BLUE);
				}
			}

		}

		for (auto &o : rObjects)
		{
			o.x += o.velX;
			o.y += o.velY;

			if (map.c_str()[(int)o.x * mapWidth + (int)o.y] == '#')
				o.toDestroy = true;

			
			
			//is the object in the FOV
			Vector2 vec(o.x - playerPos.x, o.y - playerPos.y);
			float distance = vec.magnitude();

			Vector2 eye = Vector2(sinf(playerLookAngle),cosf(playerLookAngle));
			float objectAngle = atan2f(eye.y, eye.x) - atan2f(vec.y, vec.x);

			if (objectAngle < -3.14159)
				objectAngle+= 2.0f * 3.14159;
			if (objectAngle > 3.14159)
				objectAngle-= 2.0f * 3.14159;

			bool canSee = fabs(objectAngle) < FOV / 2.0f;

			if (canSee && distance >= 0.5f && distance < maxRayDistance)
			{
				float objectCeiling = (float)(windowHeight / 2.0) - windowHeight / (float)distance;
				float objectFloor = windowHeight - objectCeiling;
				float objectHeight = objectFloor - objectCeiling;
				float aspectRatio = (float)o.sprite->height / (float)o.sprite->width;
				float objectWidth = objectHeight / aspectRatio;
				float objectMid = (0.5f * (objectAngle / (FOV / 2.0f) + 0.5f) * (float)windowWidth);

				for (float x = 0; x < objectWidth; x++)
				{
					for (float y = 0; y < objectHeight; y++)
					{
						//sample texture
						wchar_t c = o.sprite->SampleGlyph(x / objectWidth, y / objectHeight);
						int objectColumn = (int)(objectMid + x - (objectWidth / 2.0f));
						if (objectColumn >= 0 && objectColumn < windowWidth)
						{
							if (c != L' ' && depthBuffer[objectColumn] >= distance)
							{
								Draw(objectColumn, objectCeiling + y, c, o.sprite->SampleColour(x / objectWidth, y / objectHeight));
							}
						}
					}
				}
			}
		}
		rObjects.remove_if([](renderObjects &o) { return o.toDestroy; });

		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
			{
				Draw(x + 1, y + 1, map[y*mapWidth + x]);
			}
		}
		Draw(1 + (int)playerPos.y, 1 + (int)playerPos.x, L'P');

	}
};

int main()
{
	ConsoleFPS game;
	game.ConstructConsole(320, 240, 4, 4);
	game.StartThread();
	return 0;
}