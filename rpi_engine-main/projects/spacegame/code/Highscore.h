#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

class HighScore
{
public:
	int score;
	HighScore();

	// For sorting in descending order
	bool operator<(HighScore& other);
	friend std::ostream& operator<<(std::ostream& os, HighScore& hs);
	friend std::istream& operator>>(std::istream& is, HighScore& hs);
	
};

class HighScoreManager
{
public:
	std::vector<HighScore> scores;

	void sortScores();
	void loadFromFile(std::string filePath);
	void saveToFile(std::string filePath);
	void addScore(HighScore newScore); // add to score and sort
};

