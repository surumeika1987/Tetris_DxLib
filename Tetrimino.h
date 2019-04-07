#pragma once
class Tetrimino
{
private:
	std::vector<std::vector<bool>> tetriminoArray;
	int posX;
	int posY;
	int typeID;
	int dir;
public:
	int getSize();
	std::vector<std::vector<bool>> getInitTetrimino();

	Tetrimino(int posX, int posY, int typeID);

	void setPos(int posX, int posY);

	int getPosX();
	int getPosY();
	int getTypeID();
	int getDir();

	std::vector<std::vector<bool>> getTetriminoArray();

	void leftRotation();
	void rightRotation();

	Tetrimino copy();
};