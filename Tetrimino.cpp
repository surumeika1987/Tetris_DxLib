#include <vector>
#include "Tetrimino.h"

int Tetrimino::getSize()
{
	if (typeID == 0)
	{
		return 4;
	}
	else if (typeID == 1)
	{
		return 2;
	}
	else if (typeID < 7)
	{
		return 3;
	}
	else
	{
		return 1;
	}
}

std::vector<std::vector<bool>> Tetrimino::getInitTetrimino()
{
	std::vector<std::vector<bool>> tetrimino(getSize(), std::vector<bool>(getSize()));

	switch (getTypeID())
	{
	case 0:
		tetrimino[1][0] = true;
		tetrimino[1][1] = true;
		tetrimino[1][2] = true;
		tetrimino[1][3] = true;
		return tetrimino;
	case 1:
		tetrimino[0][0] = true;
		tetrimino[0][1] = true;
		tetrimino[1][0] = true;
		tetrimino[1][1] = true;
		return tetrimino;
	case 2:
		tetrimino[0][1] = true;
		tetrimino[0][2] = true;
		tetrimino[1][0] = true;
		tetrimino[1][1] = true;
		return tetrimino;
	case 3:
		tetrimino[0][0] = true;
		tetrimino[0][1] = true;
		tetrimino[1][1] = true;
		tetrimino[1][2] = true;
		return tetrimino;
	case 4:
		tetrimino[0][0] = true;
		tetrimino[1][0] = true;
		tetrimino[1][1] = true;
		tetrimino[1][2] = true;
		return tetrimino;
	case 5:
		tetrimino[0][2] = true;
		tetrimino[1][0] = true;
		tetrimino[1][1] = true;
		tetrimino[1][2] = true;
		return tetrimino;
	case 6:
		tetrimino[0][1] = true;
		tetrimino[1][0] = true;
		tetrimino[1][1] = true;
		tetrimino[1][2] = true;
		return tetrimino;
	default:
		return tetrimino;
	}
}

Tetrimino::Tetrimino(int posX, int posY, int typeID)
{
	this->posX = posX;
	this->posY = posY;
	this->typeID = typeID;

	this->dir = 0;

	this->tetriminoArray = this->getInitTetrimino();
}

void Tetrimino::setPos(int posX, int posY)
{
	this->posX = posX;
	this->posY = posY;
}

int Tetrimino::getPosX() { return this->posX; }
int Tetrimino::getPosY() { return this->posY; }
int Tetrimino::getTypeID() { return this->typeID; }
int Tetrimino::getDir() { return this->dir; }

std::vector<std::vector<bool>> Tetrimino::getTetriminoArray() { return this->tetriminoArray; }

void Tetrimino::leftRotation()
{
	std::vector<std::vector<bool>> cptetrimino(getSize(), std::vector<bool>(getSize()));
	
	for (int i = 0; i < getSize(); i++)
	{
		for (int j = 0; j < getSize(); j++)
		{
			cptetrimino[i][j] = getTetriminoArray()[j][getSize() - i - 1];
		}
	}

	this->tetriminoArray = cptetrimino;

	if (getDir() == 0) this->dir = 3;
	else this->dir -= 1;
}

void Tetrimino::rightRotation()
{
	std::vector<std::vector<bool>> cptetrimino(getSize(), std::vector<bool>(getSize()));

	for (int i = 0; i < getSize(); i++)
	{
		for (int j = 0; j < getSize(); j++)
		{
			cptetrimino[i][j] = getTetriminoArray()[getSize() - j - 1][i];
		}
	}

	this->tetriminoArray = cptetrimino;

	if (getDir() == 3) this->dir = 0;
	else this->dir += 1;
}

Tetrimino Tetrimino::copy()
{
	Tetrimino cpTetrimino = Tetrimino(getPosX(), getPosY(), getTypeID());

	for (int i = 0; i < getDir(); i++)
	{
		cpTetrimino.rightRotation();
	}
	return cpTetrimino;
}