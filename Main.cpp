#include "DxLib.h"
#include <random>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <iomanip>

#include "Tetrimino.h"

void DrawFrame(int dqFontHandle);

void DrawTetris(int fieldArray[25][10], Tetrimino tetrimino);
void DrawTetriminoQueue(std::queue<int> tetriminoQueue);
void DrawScore(int score, int fontHandle);
void DrawHold(int holdType);

bool CanMoveDown(int fieldArray[25][10], Tetrimino tetrimino);
bool CanMoveLeft(int fieldArray[25][10], Tetrimino tetrimino);
bool CanMoveRight(int fieldArray[25][10], Tetrimino tetrimino);
int DeleteBlock(int fieldArray[25][10]);
void DropAllBlock(int fieldArray[25][10]);
int CanLeftRoll(int fieldArray[25][10], Tetrimino tetrimino, int srs);
int CanRightRoll(int fieldArray[25][10], Tetrimino tetrimino, int srs);

std::vector<int> GetLeftRollSRSPos(int srs, Tetrimino tetrimino);
std::vector<int> GetRightRollSRSPos(int srs, Tetrimino tetrimino);

void MergeToArray(int fieldArray[25][10], Tetrimino tetrimino);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 変数宣言エリア
	int dqFontHandle;
	int scoreFontHandle;
	int bgmHandle;

	// テトリミノの集合
	int org_data[] = { 0, 1, 2, 3, 4, 5, 6 };
	std::vector<int> tetriminoSet(org_data, std::end(org_data));
	// テトリミノの待ち列
	std::queue<int> tetriminoQueue;

	int y = 0;
	int startCnt;
	bool isDrop;
	Tetrimino tetrimino = Tetrimino(0, 0, -1);
	// 縦に少し余裕を持たせる
	// 描画範囲は5 ～ 24
	int fieldArray[25][10];

	int score;

	int holdTypeID;
	bool isUseHold;

	bool onePressFlag;
	bool rollOnePressFlag;
	bool holdOnePressFlag;

	// ウィンドウ各種設定
	ChangeWindowMode(TRUE);
	SetMainWindowText("Tetris");
	SetWindowSize(400, 500);
	//SetWindowIconID();

	// DxLib初期化
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	// データ読み込み
	AddFontResource("DragonQuestFCIntact.ttf");
	AddFontResource("DSEG7Classic-Bold.ttf");
	dqFontHandle = CreateFontToHandle("DragonQuestFCIntact", -1, -1, -1);
	scoreFontHandle = CreateFontToHandle("DSEG7 Classic", -1, -1, -1);
	bgmHandle = LoadSoundMem("korobushka.wav");

	// 枠の生成
	DrawFrame(dqFontHandle);

	// 変数初期化
	startCnt = GetNowCount();
	isDrop = false;
	score = 0;
	holdTypeID = -1;
	isUseHold = false;
	onePressFlag = false;
	rollOnePressFlag = false;
	holdOnePressFlag = false;

	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			fieldArray[i][j] = 0;
		}
	}

	// BGM再生
	ChangeVolumeSoundMem(255 * 0.4, bgmHandle);
	PlaySoundMem(bgmHandle, DX_PLAYTYPE_LOOP);

	// 待ち列の構築
	for (int i = 0; i < 3; i++)
	{
		// 配列シャッフル
		std::random_device get_rand_dev;
		std::mt19937 get_rand_mt(get_rand_dev()); // seedに乱数を指定
		std::shuffle(tetriminoSet.begin(), tetriminoSet.end(), get_rand_mt);

		for (int j = 0; j < 7; j++)
		{
			tetriminoQueue.push(tetriminoSet[j]);
		}
	}

	// メインループ
	while (true)
	{	
		// 0.5秒ごとにテトリスを更新
		if (500 < GetNowCount() - startCnt)
		{
			if (isDrop)
			{
				// 落下可能か調べる
				if (CanMoveDown(fieldArray, tetrimino))
				{
					//可能なら落下する
					tetrimino.setPos(tetrimino.getPosX(), tetrimino.getPosY() + 1);
				}
				else
				{
					// 不可能なら落下を終える
					isDrop = false;
				}
				startCnt = GetNowCount();
			}
		}

		// 落下中なら操作を受け付ける
		if (isDrop)
		{

			// 左右の操作処理
			if (CheckHitKey(KEY_INPUT_LEFT) == 1)
			{
				if (!onePressFlag)
				{
					if (CanMoveLeft(fieldArray, tetrimino))
					{
						tetrimino.setPos(tetrimino.getPosX() - 1, tetrimino.getPosY());
						// インフィニティ(遊び時間リセット)
						if (CanMoveDown(fieldArray, tetrimino) == false) startCnt = GetNowCount();
					}
					onePressFlag = true;
				}
			}

			if (CheckHitKey(KEY_INPUT_RIGHT) == 1)
			{
				if (!onePressFlag)
				{
					if (CanMoveRight(fieldArray, tetrimino))
					{
						tetrimino.setPos(tetrimino.getPosX() + 1, tetrimino.getPosY());
						// インフィニティ(遊び時間リセット)
						if (CanMoveDown(fieldArray, tetrimino) == false) startCnt = GetNowCount();
					}
					onePressFlag = true;
				}
			}

			// 下キー(高速落下)の操作処理
			if (CheckHitKey(KEY_INPUT_DOWN) == 1)
			{
				if (!onePressFlag)
				{
					startCnt -= 1000;
					onePressFlag = true;
				}
			}

			// 左右下キーが押されていなければフラグを初期化
			if ((CheckHitKey(KEY_INPUT_LEFT) == 0) & (CheckHitKey(KEY_INPUT_RIGHT) == 0) & (CheckHitKey(KEY_INPUT_DOWN) == 0))
			{
				if (onePressFlag) onePressFlag = false;
			}

			// 左回転の操作
			if (CheckHitKey(KEY_INPUT_Z) == 1)
			{
				if (!rollOnePressFlag)
				{
					int srsResult = CanLeftRoll(fieldArray, tetrimino, 0);
					if (srsResult != -1)
					{
						tetrimino.leftRotation();

						std::vector<int> srsPos = GetLeftRollSRSPos(srsResult, tetrimino);
						tetrimino.setPos(srsPos[0], srsPos[1]);
					}
					rollOnePressFlag = true;
					// インフィニティ(遊び時間リセット)
					if (CanMoveDown(fieldArray, tetrimino) == false) startCnt = GetNowCount();
				}
			}

			// 右回転の操作
			if (CheckHitKey(KEY_INPUT_X) == 1)
			{
				if (!rollOnePressFlag)
				{
					int srsResult = CanRightRoll(fieldArray, tetrimino, 0);
					if (srsResult != -1)
					{
						tetrimino.rightRotation();

						std::vector<int> srsPos = GetRightRollSRSPos(srsResult, tetrimino);
						tetrimino.setPos(srsPos[0], srsPos[1]);
					}
					rollOnePressFlag = true;
					// インフィニティ(遊び時間リセット)
					if (CanMoveDown(fieldArray, tetrimino) == false) startCnt = GetNowCount();
				}
			}

			// 回転キーが押されていなければフラグを初期化
			if ((CheckHitKey(KEY_INPUT_Z) == 0) & (CheckHitKey(KEY_INPUT_X) == 0))
			{
				if (rollOnePressFlag) rollOnePressFlag = false;
			}

			// ホールドの操作
			if (CheckHitKey(KEY_INPUT_C) == 1)
			{
				if (!holdOnePressFlag)
				{
					if (!isUseHold)
					{
						if (holdTypeID == -1)
						{
							holdTypeID = tetrimino.getTypeID();
							
							int tid = tetriminoQueue.front();
							tetrimino = Tetrimino(2, 5, tid);

							tetriminoQueue.pop();							
						}
						else
						{
							int tid = holdTypeID;
							holdTypeID = tetrimino.getTypeID();
							tetrimino = Tetrimino(2, 5, tid);
						}
						isUseHold = true;
					}
				}
			}

			// ホールドキーが押されていなければフラグを初期化
			if (CheckHitKey(KEY_INPUT_C) == 0)
			{
				if (holdOnePressFlag) holdOnePressFlag = false;
			}
		}

		// ブロックを固定
		if (!isDrop)
		{
			MergeToArray(fieldArray, tetrimino);
		}

		// 落下終了後のブロックの消去判定
		if (!isDrop)
		{
			// 横に埋め尽くされているブロックは消す
			int deleteLineCnt = DeleteBlock(fieldArray);
			// 浮いているブロックを下に敷き詰める
			DropAllBlock(fieldArray);

			score += 100 * deleteLineCnt;

		}

		// 落下中のテトリミノがなければ新しく生成
		if (!isDrop)
		{
			int tid = tetriminoQueue.front();
			tetrimino = Tetrimino(2, 5, tid);

			tetriminoQueue.pop();

			isDrop = true;
			isUseHold = false;
		}

		// テトリミノ待機列が14以下になったら追加する
		if (tetriminoQueue.size() <= 14)
		{
			// 配列シャッフル
			std::random_device get_rand_dev;
			std::mt19937 get_rand_mt(get_rand_dev()); // seedに乱数を指定
			std::shuffle(tetriminoSet.begin(), tetriminoSet.end(), get_rand_mt);

			for (int j = 0; j < 7; j++)
			{
				tetriminoQueue.push(tetriminoSet[j]);
			}
		}

		// テトリス画面の更新
		DrawTetris(fieldArray, tetrimino);
		// テトリミノ待機列画面の更新
		DrawTetriminoQueue(tetriminoQueue);
		// ホールド画面の更新
		if (holdTypeID != -1)
		{
			DrawHold(holdTypeID);
		}
		// スコア画面の更新
		DrawScore(score, scoreFontHandle);

		// ESCキーで終了 又は エラー発生で終了
		if ((CheckHitKey(KEY_INPUT_ESCAPE) == 1) | (ProcessMessage() == -1))
		{
			break;
		}
	}

	// データのアンロード
	RemoveFontResource("DragonQuestFC.ttf");
	RemoveFontResource("DSEG7Classic-Bold.ttf");
	DeleteSoundMem(bgmHandle);

	DxLib_End();

	return 0;
}

void DrawFrame(int dqFontHandle)
{
	int testFileHandle;
	char line[256];
	int y = 0;
	
	testFileHandle = FileRead_open("frame.txt");

	while (FileRead_eof(testFileHandle) == 0)
	{
		FileRead_gets(line, 256, testFileHandle);
		DrawStringToHandle(0, y, line, GetColor(255, 255, 255), dqFontHandle);
		y += 16;
	}
}

void DrawTetriminoQueue(std::queue<int> tetriminoQueue)
{
	std::queue<int> cpQueue = tetriminoQueue;

	int queueFieldArray[21][4];

	for (int i = 0; i < 21; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			queueFieldArray[i][j] = 0;
		}
	}

	// テトリミノ待機列用のフィールド
	// (描画処理はテトリス画面表示を流用)
	for (int i = 0; i < 7; i++)
	{
		int id = cpQueue.front();

		int vy = (3 * i);

		Tetrimino tet = Tetrimino(0, vy, id);

		for (int i = 0; i < tet.getSize(); i++)
		{
			for (int j = 0; j < tet.getSize(); j++)
			{
				if (tet.getTetriminoArray()[i][j])
				{
					queueFieldArray[tet.getPosY() + i][tet.getPosX() + j] = tet.getTypeID() + 1;
				}
			}
		}

		cpQueue.pop();
	}

	int x = 480;
	int y = 30;
	int vx = 27;
	int vy = 14;

	DrawBox(x, 20, x + (vx * 4), y + (vy * 21), GetColor(0, 0, 0), TRUE);

	for (int i = 0; i < 21; i++)
	{
		for (int j = 0; j < 4; j++) {
			if (queueFieldArray[i][j] != 0)
			{
				int px = x + (vx * j);
				int py = y + (vy * (i));
				DrawBox(px, py, px + 25, py + 12, GetColor(255, 255, 255), TRUE);
			}
		}
	}
}

void DrawTetris(int fieldArray[25][10], Tetrimino tetrimino) {
	int x = 18;
	int y = 12;
	int vx = 39;
	int vy = 23;

	int cpArray[25][10];
	
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			cpArray[i][j] = fieldArray[i][j];
		}
	}

	MergeToArray(cpArray, tetrimino);

	DrawBox(x, y, x + (vx * 10), y + (vy * 20), GetColor(0, 0, 0), TRUE);

	for (int i = 5; i < 25; i++)
	{
		for (int j = 0; j < 10; j++) {
			if (cpArray[i][j] != 0)
			{
				int px = x + (vx * j);
				int py = y + (vy * (i - 5));
				DrawBox(px, py, px + 37, py + 21, GetColor(255, 255, 255), TRUE);
			}
		}
	}
}

void DrawScore(int score, int fontHandle)
{

	std::ostringstream str;
	str << std::setw(10) << std::setfill('0') << score;

	// 前の値を削除
	DrawBox(460, 450, 610, 470, GetColor(0, 0, 0), TRUE);

	DrawStringToHandle(460, 450, str.str().c_str(), GetColor(255, 255, 255), fontHandle);
}

void DrawHold(int holdType)
{
	int holdFieldArray[2][4];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			holdFieldArray[i][j] = 0;
		}
	}

	switch (holdType)
	{
	case 0:
		holdFieldArray[0][0] = 1;
		holdFieldArray[0][1] = 1;
		holdFieldArray[0][2] = 1;
		holdFieldArray[0][3] = 1;
		break;
	case 1:
		holdFieldArray[0][0] = 2;
		holdFieldArray[0][1] = 2;
		holdFieldArray[1][0] = 2;
		holdFieldArray[1][1] = 2;
		break;
	case 2:
		holdFieldArray[0][1] = 3;
		holdFieldArray[0][2] = 3;
		holdFieldArray[1][0] = 3;
		holdFieldArray[1][1] = 3;
		break;
	case 3:
		holdFieldArray[0][0] = 4;
		holdFieldArray[0][1] = 4;
		holdFieldArray[1][1] = 4;
		holdFieldArray[1][2] = 4;
		break;
	case 4:
		holdFieldArray[0][0] = 5;
		holdFieldArray[1][0] = 5;
		holdFieldArray[1][1] = 5;
		holdFieldArray[1][2] = 5;
		break;
	case 5:
		holdFieldArray[0][2] = 6;
		holdFieldArray[1][0] = 6;
		holdFieldArray[1][1] = 6;
		holdFieldArray[1][2] = 6;
		break;
	case 6:
		holdFieldArray[0][1] = 7;
		holdFieldArray[1][0] = 7;
		holdFieldArray[1][1] = 7;
		holdFieldArray[1][2] = 7;
		break;
	}

	int x = 485;
	int y = 375;
	int vx = 27;
	int vy = 14;

	DrawBox(x, y, x + (vx * 4), y + (vy * 2), GetColor(0, 0, 0), TRUE);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++) {
			if (holdFieldArray[i][j] != 0)
			{
				int px = x + (vx * j);
				int py = y + (vy * (i));
				DrawBox(px, py, px + 25, py + 12, GetColor(255, 255, 255), TRUE);
			}
		}
	}
}

bool CanMoveDown(int fieldArray[25][10], Tetrimino tetrimino)
{
	int vx = tetrimino.getPosX();
	int vy = tetrimino.getPosY() + 1;

	for (int i = 0; i < tetrimino.getSize(); i++)
	{
		for (int j = 0; j < tetrimino.getSize(); j++)
		{
			if (tetrimino.getTetriminoArray()[i][j] == true)
			{
				if (24 < vy + i) { return false; }
				if (fieldArray[vy + i][vx + j] != 0) { return false; }
			}
		}
	}
	return true;
}

bool CanMoveLeft(int fieldArray[25][10], Tetrimino tetrimino)
{
	int vx = tetrimino.getPosX() - 1;
	int vy = tetrimino.getPosY();

	for (int i = 0; i < tetrimino.getSize(); i++)
	{
		for (int j = 0; j < tetrimino.getSize(); j++)
		{
			if (tetrimino.getTetriminoArray()[i][j] == true)
			{
				if (vx + j < 0) { return false; }
				if (fieldArray[vy + i][vx + j] != 0) { return false; }
			}
		}
	}
	return true;
}

bool CanMoveRight(int fieldArray[25][10], Tetrimino tetrimino)
{
	int vx = tetrimino.getPosX() + 1;
	int vy = tetrimino.getPosY();

	for (int i = 0; i < tetrimino.getSize(); i++)
	{
		for (int j = 0; j < tetrimino.getSize(); j++)
		{
			if (tetrimino.getTetriminoArray()[i][j] == true)
			{
				if (9 < vx + j) { return false; }
				if (fieldArray[vy + i][vx + j] != 0) { return false; }
			}
		}
	}
	return true;
}

int DeleteBlock(int fieldArray[25][10])
{
	int cnt = 0;
	for (int i = 0; i < 25; i++)
	{
		bool isRemove = true;
		for (int j = 0; j < 10; j++)
		{
			if (fieldArray[i][j] == 0)
			{
				isRemove = false;
				break;
			}
		}
		if (isRemove)
		{
			for (int j = 0; j < 10; j++)
			{
				fieldArray[i][j] = 0;
				cnt += 1;
			}
		}
	}
	
	return cnt;
}

void DropAllBlock(int fieldArray[25][10])
{
	int y = 0;
	for (int i = 24; 0 <= i; i--)
	{
		bool isDrop = true;
		for (int j = 0; j < 10; j++)
		{
			if (fieldArray[i][j] != 0)
			{
				isDrop = false;
				break;
			}
		}
		if (isDrop)
		{
			// 一列が空ならyに1を足す
			y += 1;
		}
		else
		{
			// 空洞ではないのなら敷き詰める
			if (y != 0)
			{
				for (int j = 0; j < 10; j++)
				{
					if (fieldArray[i][j] != 0)
					{
						int id = fieldArray[i][j];
						fieldArray[i][j] = 0;
						
						fieldArray[i + y][j] = id;
					}
				}
			}
		}
	}
}

int CanLeftRoll(int fieldArray[25][10], Tetrimino tetrimino, int srs)
{
	if (srs == 5) return -1;

	Tetrimino cpTetrimino = tetrimino.copy();

	cpTetrimino.leftRotation();

	std::vector<int> srsPos = GetLeftRollSRSPos(srs, cpTetrimino);
	cpTetrimino.setPos(srsPos[0], srsPos[1]);

	for (int i = 0; i < cpTetrimino.getSize(); i++)
	{
		for (int j = 0; j < cpTetrimino.getSize(); j++)
		{
			if (cpTetrimino.getTetriminoArray()[i][j])
			{
				if (((cpTetrimino.getPosY() + i < 0) | (24 < cpTetrimino.getPosY() + i)) | ((cpTetrimino.getPosX() + j < 0) | (9 < cpTetrimino.getPosX() + j)))
				{
					return CanLeftRoll(fieldArray, tetrimino, srs + 1);
				}

				if (fieldArray[cpTetrimino.getPosY() + i][cpTetrimino.getPosX() + j] != 0)
				{
					return CanLeftRoll(fieldArray, tetrimino, srs + 1);
				}
			}
		}
	}
	
	return srs;
}

int CanRightRoll(int fieldArray[25][10], Tetrimino tetrimino, int srs)
{
	if (srs == 5) return -1;

	Tetrimino cpTetrimino = tetrimino.copy();

	cpTetrimino.rightRotation();
	std::vector<int> srsPos = GetRightRollSRSPos(srs, cpTetrimino);
	cpTetrimino.setPos(srsPos[0], srsPos[1]);

	for (int i = 0; i < cpTetrimino.getSize(); i++)
	{
		for (int j = 0; j < cpTetrimino.getSize(); j++)
		{
			if (cpTetrimino.getTetriminoArray()[i][j])
			{
				if (((cpTetrimino.getPosY() + i < 0) | (24 < cpTetrimino.getPosY() + i)) | ((cpTetrimino.getPosX() + j < 0) | (9 < cpTetrimino.getPosX() + j)))
				{
					return CanRightRoll(fieldArray, tetrimino, srs + 1);
				}

				if (fieldArray[cpTetrimino.getPosY() + i][cpTetrimino.getPosX() + j] != 0)
				{
					return CanRightRoll(fieldArray, tetrimino, srs + 1);
				}
			}
		}
	}

	return srs;
}

std::vector<int> GetLeftRollSRSPos(int srs, Tetrimino tetrimino)
{
	int posData[] = { 0, 0 };
	posData[0] = tetrimino.getPosX();
	posData[1] = tetrimino.getPosY();

	if (tetrimino.getTypeID() == 0)
	{
		// Iミノ用
		switch (srs)
		{
		case 1:
			if (tetrimino.getDir() == 0) { posData[0] += 2; }
			else if (tetrimino.getDir() == 1) { posData[0] += 1; }
			else if (tetrimino.getDir() == 2) { posData[0] += 1; }
			else if (tetrimino.getDir() == 3) { posData[0] -= 1; }
			break;
		case 2:
			if (tetrimino.getDir() == 0) { posData[0] -= 1; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 2; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[0] += 2; }
			break;
		case 3:
			if (tetrimino.getDir() == 0) { posData[0] += 2; posData[1] -= 1; }
			else if (tetrimino.getDir() == 1) { posData[0] += 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 2; posData[1] += 1; }
			else if (tetrimino.getDir() == 3) { posData[0] -= 1; posData[1] -= 2; }
			break;
		case 4:
			if (tetrimino.getDir() == 0) { posData[0] -= 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 2; posData[1] -= 1; }
			else if (tetrimino.getDir() == 2) { posData[0] += 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[0] += 2; posData[1] += 1; }
			break;
		}
	}
	else
	{
		// Iミノ以外
		switch (srs)
		{
		case 1:
			if (tetrimino.getDir() == 0) { posData[0] += 1; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 1; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 1; }
			else if (tetrimino.getDir() == 3) { posData[0] += 1; }
			break;
		case 2:
			if (tetrimino.getDir() == 0) { posData[0] += 1; posData[1] += 1; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 1; posData[1] -= 1; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 1; posData[1] += 1; }
			else if (tetrimino.getDir() == 3) { posData[0] += 1; posData[1] -= 1; }
			break;
		case 3:
			if (tetrimino.getDir() == 0) { posData[1] -= 2; }
			else if (tetrimino.getDir() == 1) { posData[1] += 2; }
			else if (tetrimino.getDir() == 2) { posData[1] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[1] += 2; }
			break;
		case 4:
			if (tetrimino.getDir() == 0) { posData[0] += 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 1; posData[1] += 2; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[0] += 1; posData[1] += 2; }
			break;
		}
	}

	std::vector<int> returnData(posData, std::end(posData));
	return returnData;
}

std::vector<int> GetRightRollSRSPos(int srs, Tetrimino tetrimino)
{
	int posData[] = { 0, 0 };
	posData[0] = tetrimino.getPosX();
	posData[1] = tetrimino.getPosY();

	if (tetrimino.getTypeID() == 0)
	{
		// Iミノ用
		switch (srs)
		{
		case 1:
			if (tetrimino.getDir() == 0) { posData[0] -= 2; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 2; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 1; }
			else if (tetrimino.getDir() == 3) { posData[0] += 2; }
			break;
		case 2:
			if (tetrimino.getDir() == 0) { posData[0] += 1; }
			else if (tetrimino.getDir() == 1) { posData[0] += 1; }
			else if (tetrimino.getDir() == 2) { posData[0] += 2; }
			else if (tetrimino.getDir() == 3) { posData[0] -= 1; }
			break;
		case 3:
			if (tetrimino.getDir() == 0) { posData[0] += 1; posData[1] += 2; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 2; posData[1] += 1; }
			else if (tetrimino.getDir() == 2) { posData[0] -= 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[0] += 2; posData[1] -= 1; }
			break;
		case 4:
			if (tetrimino.getDir() == 0) { posData[0] -= 2; posData[1] -= 1; }
			else if (tetrimino.getDir() == 1) { posData[0] += 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 2) { posData[0] += 2; posData[1] += 1; }
			else if (tetrimino.getDir() == 3) { posData[0] -= 1; posData[1] += 2; }
			break;
		}
	}
	else
	{
		// Iミノ以外
		switch (srs)
		{
		case 1:
			if (tetrimino.getDir() == 0) { posData[0] -= 1; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 1; }
			else if (tetrimino.getDir() == 2) { posData[0] += 1; }
			else if (tetrimino.getDir() == 3) { posData[0] += 1; }
			break;
		case 2:
			if (tetrimino.getDir() == 0) { posData[0] -= 1; posData[1] += 1; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 1; posData[1] -= 1; }
			else if (tetrimino.getDir() == 2) { posData[0] += 1; posData[1] += 1; }
			else if (tetrimino.getDir() == 3) { posData[0] += 1; posData[1] -= 1; }
			break;
		case 3:
			if (tetrimino.getDir() == 0) { posData[1] -= 2; }
			else if (tetrimino.getDir() == 1) { posData[1] += 2; }
			else if (tetrimino.getDir() == 2) { posData[1] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[1] += 2; }
			break;
		case 4:
			if (tetrimino.getDir() == 0) { posData[0] -= 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 1) { posData[0] -= 1; posData[1] += 2; }
			else if (tetrimino.getDir() == 2) { posData[0] += 1; posData[1] -= 2; }
			else if (tetrimino.getDir() == 3) { posData[0] += 1; posData[1] += 2; }
			break;
		}
	}

	std::vector<int> returnData(posData, std::end(posData));
	return returnData;
}

void MergeToArray(int fieldArray[25][10], Tetrimino tetrimino)
{
	for (int i = 0; i < tetrimino.getSize(); i++)
	{
		for (int j = 0; j < tetrimino.getSize(); j++)
		{
			if (tetrimino.getTetriminoArray()[i][j])
			{
				fieldArray[tetrimino.getPosY() + i][tetrimino.getPosX() + j] = tetrimino.getTypeID() + 1;
			}
		}
	}
}