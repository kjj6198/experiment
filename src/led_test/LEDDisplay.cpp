#include "LEDDisplay.h"
#include "LedControl.h"

int mat[64] = {
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1};

LEDDisplay::LEDDisplay(int dataPin, int clkPin, int csPin)
{
	this->lc = LedControl(dataPin, clkPin, csPin, 1);
}

LEDDisplay::~LEDDisplay()
{
}

void LEDDisplay::init()
{
	this->lc.shutdown(0, false);
	this->lc.setIntensity(0, 0);
	this->lc.clearDisplay(0);
}

void LEDDisplay::zeros()
{
	for (int i = 0; i < 64; i++)
	{
		mat[i] = 0;
	}
}

void LEDDisplay::fill(int row, int col, int val)
{
	mat[i * 8 + col] = val;
	this->lc.setLed(0, row, 7 - col, val);
}

void LEDDisplay::render()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			lc.setLed(0, i, 7 - j, mat[i * 8 + j]);
		}
	}
}
