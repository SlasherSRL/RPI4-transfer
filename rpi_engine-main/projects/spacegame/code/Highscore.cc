#include "Highscore.h"

HighScore::HighScore()
{
}

bool HighScore::operator<(HighScore& other)
{
    return score < other.score; // Note: > for descending order
}


void HighScoreManager::sortScores()
{
    std::sort(scores.begin(), scores.end());
}

void HighScoreManager::loadFromFile(std::string filePath)
{
    scores.clear();
    std::ifstream file(filePath);
    if (file.is_open()) 
    {
        HighScore temp;
        while (file >> temp) 
        {
            scores.push_back(temp);
        }
        file.close();
        sortScores();
    }
}

void HighScoreManager::saveToFile(std::string filePath)
{
    std::ofstream file(filePath);
    if (file.is_open()) 
    {
        for (HighScore& hs : scores)
        {
            file << hs.score << '\n';
        }
        file.close();

    }
}

void HighScoreManager::addScore(HighScore newScore)
{
    scores.push_back(newScore);
    sortScores();
}

std::ostream& operator<<(std::ostream& os, HighScore& hs)
{
    os << hs.score;
    return os;
}

std::istream& operator>>(std::istream& is, HighScore& hs)
{
    is >> hs.score;
    return is;
}
