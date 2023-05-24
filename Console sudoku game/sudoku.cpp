#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <random>
#include <array>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <conio.h>

HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE); // used for goto
COORD CursorPosition; // used for goto

using namespace std;

const int SUDOKU_SIZE = 9;

const int EMPTY_VALUE = 0;
array<int, 9> values{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

char ch;
bool running = true;

void gotoXY(int x, int y)
{
	CursorPosition.X = x;
	CursorPosition.Y = y;
	SetConsoleCursorPosition(console, CursorPosition);
}


class Sudoku
{
private:
	int difficulty;
	int grid[SUDOKU_SIZE][SUDOKU_SIZE];
	int copy[SUDOKU_SIZE][SUDOKU_SIZE];
	bool win = false;
public:
	Sudoku(int);
	void printSudoku();
	void generateSudoku();
	void setValue(int, int, int);
	bool checkValue(int, int, int);
	bool empty(int, int);
	bool fillSudoku();
	bool isValid(int, int, int);
	bool emptyCell();
	void copyGrid();
	void draw(int, int);
	void game();
	bool checkWin();
};

/*
*  -----------------------
* | 1 2 3 | 4 5 6 | 7 8 9 |
* | 1 2 3 | 4 5 6 | 7 8 9 |
* | 1 2 3 | 4 5 6 | 7 8 9 |
*  -----------------------
* | 1 2 3 | 4 5 6 | 7 8 9 |
* | 1 2 3 | 4 5 6 | 7 8 9 |
* | 1 2 3 | 4 5 6 | 7 8 9 |
*  -----------------------
* | 1 2 3 | 4 5 6 | 7 8 9 |
* | 1 2 3 | 4 5 6 | 7 8 9 |
* | 1 2 3 | 4 5 6 | 7 8 9 |
*  -----------------------
*/

Sudoku::Sudoku(int diff)
{
	for (int i = 0; i < SUDOKU_SIZE; i++)
		for (int j = 0; j < SUDOKU_SIZE; j++)
			grid[i][j] = EMPTY_VALUE;

	difficulty = diff;

}

bool Sudoku::checkValue(int x, int y, int v)
{
	if (copy[x][y] == v) return true;
	else return false;
}

void Sudoku::setValue(int x, int y, int v)
{
	grid[x][y] = v;
}

bool Sudoku::empty(int x, int y)
{
	if (grid[x][y] == EMPTY_VALUE) return true;
	else return false;
}

bool Sudoku::isValid(int value, int row, int col)
{
	for (int c = 0; c < SUDOKU_SIZE; c++)
	{
		if (grid[row][c] == value) return false;
	}
	for (int r = 0; r < SUDOKU_SIZE; r++)
	{
		if (grid[r][col] == value) return false;
	}

	int startRow = floor(row / 3) * 3, startCol = floor(col / 3) * 3;

	for (int r = startRow; r < startRow + 3; r++)
	{
		for (int c = startCol; c < startCol + 3; c++)
		{
			if (grid[r][c] == value)
				return false;
		}
	}
	return true;
}

bool Sudoku::emptyCell()
{
	for (int r = 0; r < SUDOKU_SIZE; r++)
	{
		for (int c = 0; c < SUDOKU_SIZE; c++)
		{
			if (grid[r][c] == EMPTY_VALUE)
				return true;
		}
	}
	return false;
}

void Sudoku::copyGrid()
{
	for (int r = 0; r < SUDOKU_SIZE; r++)
	{
		for (int c = 0; c < SUDOKU_SIZE; c++)
		{
			copy[r][c] = grid[r][c];
		}
	}
}

bool Sudoku::fillSudoku()
{
	int row, col;

	for (int i = 0; i < SUDOKU_SIZE * SUDOKU_SIZE; i++)
	{
		row = floor(i / SUDOKU_SIZE);
		col = i % SUDOKU_SIZE;
		if (grid[row][col] == EMPTY_VALUE)
		{
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			std::shuffle(values.begin(), values.end(), std::default_random_engine(seed));
			for (int j = 0; j < SUDOKU_SIZE; j++)
			{
				if (isValid(values[j], row, col))
				{
					grid[row][col] = values[j];
					if (!emptyCell() || fillSudoku())
					{
						return true;
					}
				}
			}
			break;
		}
	}
	grid[row][col] = EMPTY_VALUE;
	return false;
}

void Sudoku::printSudoku()
{
	string separator, text;

	for (int i = 0; i < SUDOKU_SIZE; i++)
	{
		text = "	|";
		separator = "	 -";

		for (int j = 0; j < SUDOKU_SIZE; j++)
		{
			string value = grid[i][j] == EMPTY_VALUE ? " " : to_string(grid[i][j]);
			separator += "--";
			text += " " + value;

			if (j % 3 == 2 && j != SUDOKU_SIZE - 1)
			{
				separator += "-";
				text += " |";
			}
			if (j == 8)
			{
				separator += "--";
				text += " |";
			}
		}

		if (i % 3 == 0)
			cout << separator << endl;

		cout << text << endl;
	}
	cout << separator << endl;
}

void Sudoku::generateSudoku()
{
	fillSudoku();
	copyGrid();
	srand((unsigned)time(0));

	for (int i = 0; i < difficulty; i++)
	{
		int row = floor(rand() % SUDOKU_SIZE);
		int col = floor(rand() % SUDOKU_SIZE);

		while (grid[row][col] == EMPTY_VALUE)
		{
			row = floor(rand() % SUDOKU_SIZE);
			col = floor(rand() % SUDOKU_SIZE);
		}

		int backupValue = grid[row][col];
		grid[row][col] = EMPTY_VALUE;
	}

}

void Sudoku::draw(int x, int y)
{
	system("cls");
	printSudoku();
	gotoXY(x, y); cout << "";
	Sleep(100);
}

void Sudoku::game()
{
	generateSudoku();
	int y = 1;
	int x = 10;
	int row = 0, col = 0;
	draw(x, y);


	while (1)
	{
		if (GetAsyncKeyState(VK_DOWN) && y != 11) //down button pressed
		{
			y++;
			row++;
			if (y % 4 == 0) y++;
			draw(x, y);
			continue;

		}

		else if (GetAsyncKeyState(VK_UP) && y != 1) //up button pressed
		{
			y--;
			row--;
			if (y % 4 == 0) y--;
			draw(x, y);
			continue;
		}

		else if (GetAsyncKeyState(VK_LEFT) && x != 10) //down button pressed
		{
			x -= 2;
			col--;
			if (x == 16 || x == 24) x -= 2;
			draw(x, y);
			continue;

		}

		else if (GetAsyncKeyState(VK_RIGHT) && x != 30) //down button pressed
		{
			x += 2;
			col++;
			if (x == 16 || x == 24) x += 2;
			draw(x, y);
			continue;

		}

		if (_kbhit())
		{
			ch = _getch();

			if (int(ch) == 49 || int(ch) == 50 || int(ch) == 51 || int(ch) == 52 || int(ch) == 53 || int(ch) == 54 || int(ch) == 55 || int(ch) == 56 || int(ch) == 57)
			{
				if (empty(row, col))
				{
					if (checkValue(row, col, int(ch) - 48))
					{
						setValue(row, col, int(ch) - 48);
						draw(x, y);
						continue;
					}
					gotoXY(10, 15);  cout << "You lost";
					Sleep(2000);
					system("cls");
					gotoXY(18, 7); cout << "->";
					break;

				}
			}
			if (int(ch) == 27)
			{
				system("cls");
				gotoXY(18, 7); cout << "->";
				break;
			}
		}

		if (checkWin())
		{
			gotoXY(10, 15);  cout << "You won";
			Sleep(2000);
			system("cls");
			gotoXY(18, 7); cout << "->";
			break;
		}
	}
}

bool Sudoku::checkWin()
{
	for (int r = 0; r < SUDOKU_SIZE; r++)
	{
		for (int c = 0; c < SUDOKU_SIZE; c++)
		{
			if (grid[r][c] == EMPTY_VALUE)
				return false;
		}
	}
	return true;
}

int main()
{
	Sudoku sudoku(4);
	int menu_item = 0, x = 7, start = 0;

	gotoXY(18, 5); cout << "Menu";
	gotoXY(18, 7); cout << "->";

	while (running)
	{
		gotoXY(18, 5); cout << "Menu";
		gotoXY(20, 7);  cout << "New game";
		gotoXY(20, 8);  cout << "Quit Program";

		_getch();

		if (GetAsyncKeyState(VK_DOWN) && x != 8) //down button pressed
		{
			gotoXY(18, x); cout << "  ";
			x++;
			gotoXY(18, x); cout << "->";
			menu_item++;
			continue;

		}

		if (GetAsyncKeyState(VK_UP) && x != 7) //up button pressed
		{
			gotoXY(18, x); cout << "  ";
			x--;
			gotoXY(18, x); cout << "->";
			menu_item--;
			continue;
		}

		if (GetAsyncKeyState(VK_RETURN)) { // Enter key pressed

			switch (menu_item)
			{
			case 0:
				sudoku.game();
				break;
			case 1:
				running = false;
				break;
			}
		}
	}

	return 0;
}