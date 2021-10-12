/**
 * Framework for 2584 & 2584-like Games (C++ 11)
 * board.h: Define the game state and basic operations of the game of 2584
 *
 * Author: Theory of Computer Games (TCG 2021)
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <vector>

/**
 * array-based board for 2584
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */

class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 4> row;
	typedef std::array<row, 4> grid;
	typedef uint64_t data;
	typedef int reward;

public:
	board() : tile(), attr(0) {}
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 4][i % 4]; }
	const cell& operator ()(unsigned i) const { return tile[i / 4][i % 4]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:

    unsigned int getMaxIndex(){
		unsigned int  max=0;
		unsigned int maxIndex=0;
		for( unsigned int i =0;i<16;i++){
             if(tile[i / 4][i % 4]>max){
				 max=tile[i / 4][i % 4];
				 maxIndex = i;
			 }
		}
		return maxIndex;
	}

	int getOrderPoint(){
		/*
		int point = 0;
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];

			if((row[0]<=row[1] and row[1]<=row[2] and row[2]<=row[3]) or
			    (row[0]>=row[1] and row[1]>=row[2] and row[2]>=row[3])
			  )
			{
				point ++;
			}
		}
		for (int c = 0; c < 4; c++) {
			if((tile[0][c]<=tile[1][c] and tile[1][c]<=tile[2][c] and tile[2][c]<=tile[3][c]) or
			    (tile[0][c]>=tile[1][c] and tile[1][c]>=tile[2][c] and tile[2][c]>=tile[3][c])
			  )
			{
				point ++;
			}
		}
		return point;
		*/
		int point = 0;
		for (int r = 0; r < 4; r++) {
			if(r == 1 or r==2) continue;
			auto& row = tile[r];

			if((row[0]<row[1] and row[1]<row[2] and row[2]<row[3]) or
			    (row[0]>row[1] and row[1]>row[2] and row[2]>row[3])
			  )
			{
				point ++;
			}
		}
		for (int c = 0; c < 4; c++) {
			if(c == 1 or c==2) continue;
			if((tile[0][c]<tile[1][c] and tile[1][c]<tile[2][c] and tile[2][c]<tile[3][c]) or
			    (tile[0][c]>tile[1][c] and tile[1][c]>tile[2][c] and tile[2][c]>tile[3][c])
			  )
			{
				point ++;
			}
		}
		return point;
	}

	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	reward place(unsigned pos, cell tile) {
		if (pos >= 16) return -1;
		if (tile != 1 && tile != 2) return -1;
		operator()(pos) = tile;
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

	reward slide_left() {
		board prev = *this;
		reward score = 0;
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];
			int top = 0, hold = 0;
			for (int c = 0; c < 4; c++) {
				int tile = row[c];
				if (tile == 0) continue;
				row[c] = 0;
				//不是第一排
				if (hold) {
					//可以合成
					if (abs(tile-hold) == 1 or (tile == 1 and hold ==1)) {
						int newTile = tile>hold?++tile:++hold;
						row[top++] = newTile;
						score += (1 << newTile);
						hold = 0;
					//不能合成
					} else {
						//恢復tile值
						row[top++] = hold;
						// hold重新計
						hold = tile;
					} 
				//是第一排	
				} else {
					hold = tile;
				}
			}
			if (hold) tile[r][top] = hold;
		}
		return (*this != prev) ? score : -1;
	}
	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		return score;
	}
	reward slide_up() {
		rotate_right();
		reward score = slide_right();
		rotate_left();
		return score;
	}
	reward slide_down() {
		rotate_right();
		reward score = slide_left();
		rotate_left();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		std::vector<int> table{0, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025};

		out << "+------------------------+" << std::endl;
		for (auto& row : b.tile) {
			out << "|" << std::dec;
			for (auto t : row) out << std::setw(6) << table[t];
			out << "|" << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}
	friend std::istream& operator >>(std::istream& in, board& b) {
		for (int i = 0; i < 16; i++) {
			while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
			in >> b(i);
			b(i) = std::log2(b(i));
		}
		return in;
	}

private:

	grid tile;
	data attr;
};