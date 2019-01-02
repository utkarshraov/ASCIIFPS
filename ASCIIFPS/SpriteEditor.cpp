#include <iostream>
#include <string>
using namespace std;

#include "ConsoleGameEngine.h"



class SpriteEditor : public ConsoleGameEngine
{
public:
	SpriteEditor()
	{
		appName = L"Sprite Editor";
	}

private:
	int posX = 0;
	int posY = 0;
	int offsetX = 0;
	int offsetY = 0;
	int zoom = 4;
	int currentGlyph = PIXEL_SOLID;
	int currentColourFG = FG_RED;
	int currentColourBG = FG_BLACK;

	Sprite *sprite = nullptr;
	wstring CurrentSpriteFile;

public:
	virtual bool Start()
	{
		sprite = new Sprite(8, 32);
		CurrentSpriteFile = L"fireball.spr";

		return true;
	}

	virtual bool Update(float elapsedTime)
	{

		// Zooming 
		if (keys[VK_PRIOR].onReleased)
			zoom <<= 1;

		if (keys[VK_NEXT].onReleased)
			zoom >>= 1;

		if (zoom > 32) zoom = 32;
		if (zoom < 2) zoom = 2;

		// Brushes
		if (keys[VK_F1].onReleased) currentGlyph = PIXEL_SOLID;
		if (keys[VK_F2].onReleased) currentGlyph = PIXEL_THREEQUARTERS;
		if (keys[VK_F3].onReleased) currentGlyph = PIXEL_HALF;
		if (keys[VK_F4].onReleased) currentGlyph = PIXEL_QUARTER;

		// Colours
		for (int i = 0; i < 8; i++)
			if (keys[L"01234567"[i]].onReleased)
				if (keys[VK_SHIFT].onHeld)
					currentColourFG = i + 8;
				else
					currentColourFG = i;


		if (keys[VK_F7].onReleased)
			currentColourBG--;

		if (keys[VK_F8].onReleased)
			currentColourBG++;

		if (currentColourBG < 0) currentColourBG = 15;
		if (currentColourBG > 15) currentColourBG = 0;

		if (keys[VK_SHIFT].onHeld)
		{
			if (keys[VK_UP].onReleased) offsetY++;
			if (keys[VK_DOWN].onReleased) offsetY--;
			if (keys[VK_LEFT].onReleased) offsetX++;
			if (keys[VK_RIGHT].onReleased) offsetX--;
		}
		else
		{
			if (keys[VK_UP].onReleased) posY--;
			if (keys[VK_DOWN].onReleased) posY++;
			if (keys[VK_LEFT].onReleased) posX--;
			if (keys[VK_RIGHT].onReleased) posX++;
		}

		if (sprite != nullptr)
		{
			if (posX < 0) posX = 0;
			if (posX >= sprite->width) posX = sprite->width - 1;
			if (posY < 0) posY = 0;
			if (posY >= sprite->height) posY = sprite->height - 1;

			if (keys[VK_SPACE].onReleased)
			{
				sprite->SetGlyph(posX - 0, posY - 0, currentGlyph);
				sprite->SetColour(posX - 0, posY - 0, currentColourFG | (currentColourBG << 4));
			}

			if (keys[VK_DELETE].onReleased)
			{
				sprite->SetGlyph(posX - 0, posY - 0, L' ');
				sprite->SetColour(posX - 0, posY - 0, 0);
			}

			if (keys[VK_F9].onReleased)
			{
				sprite->Load(CurrentSpriteFile);
			}

			if (keys[VK_F10].onReleased)
			{
				sprite->Save(CurrentSpriteFile);
			}
		}





		// Erase All
		Fill(0, 0, windowWidth, windowHeight, L' ', 0);

		// Draw Menu
		DrawString(1, 1, L"F1 = 100%  F2 = 75%  F3 = 50%  F4 = 25%    F9 = Load File  F10 = Save File");
		for (int i = 0; i < 8; i++)
		{
			DrawString(1 + 6 * i, 3, to_wstring(i) + L" = ");
			if (keys[VK_SHIFT].onHeld)
				Draw(1 + 6 * i + 4, 3, PIXEL_SOLID, (i + 8));
			else
				Draw(1 + 6 * i + 4, 3, PIXEL_SOLID, (i));
		}

		DrawString(1, 5, L"Current Brush = ");
		Draw(18, 5, currentGlyph, currentColourFG | (currentColourBG << 4));

		// Draw Canvas
		for (int x = 9; x < 138; x++)
		{
			Draw(x, 9);
			Draw(x, 74);
		}

		for (int y = 9; y < 75; y++)
		{
			Draw(9, y);
			Draw(138, y);
		}

		// Draw Visible Sprite
		if (sprite != nullptr)
		{
			int nVisiblePixelsX = 128 / zoom;
			int nVisiblePixelsY = 64 / zoom;

			for (int x = 0; x < nVisiblePixelsX; x++)
				for (int y = 0; y < nVisiblePixelsY; y++)
				{
					if (x - offsetX < sprite->width && y - offsetY < sprite->height && x - offsetX >= 0 && y - offsetY >= 0)
					{
						// Draw Sprite
						Fill(x * zoom + 10, y*zoom + 10, (x + 1)*zoom + 10, (y + 1)*zoom + 10, sprite->GetGlyph(x - offsetX, y - offsetY), sprite->GetColour(x - offsetX, y - offsetY));


						// Draw Pixel Markers
						if (sprite->GetGlyph(x - offsetX, y - offsetY) == L' ')
							Draw((x)* zoom + 10, (y)* zoom + 10, L'.');
					}

					if (x - offsetX == posX && y - offsetY == posY)
						Draw((x)* zoom + 10, (y)* zoom + 10, L'O');
				}
		}


		// Draw Actual Sprite
		for (int x = 0; x < sprite->width; x++)
			for (int y = 0; y < sprite->height; y++)
				Draw(x + 10, y + 80, sprite->GetGlyph(x, y), sprite->GetColour(x, y));



		return true;
	}
};

//
//int main()
//{
//	SpriteEditor game;
//	game.ConstructConsole(160, 100, 8, 8);
//	game.StartThread();
//
//	return 0;
//}
