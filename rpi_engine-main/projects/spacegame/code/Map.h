#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "Object.h"
#include "Highscore.h"


class Map
{
private:
	std::vector<std::vector<char>> map;

public:
	std::vector<Object> objects;
	HighScoreManager highScoreManager;
	std::string highScorePathName;

	glm::vec3 spawnPoint;
	glm::vec3 holePos;

	unsigned int gridSizeX;
	unsigned int gridSizeZ;
	float scaleMultiplier;
	float spacing;



	Map();
	Map(float scaleMultiplier, float spacing);

	void readMapFromFile(std::string filePath);

	void settingUpMap1();
	void settingUpMap2();
	void settingUpMap3();
	//TODO map builder XD
};