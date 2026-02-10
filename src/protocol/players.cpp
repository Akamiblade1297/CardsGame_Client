#include "players.h"

Player::Player ( std::string lvl, std::string pwr, std::string gld )
  : Level{lvl},
    Power{pwr},
    Gold{gld}
{
    Color = playerColors[playerColorsDist(rng)];
}

std::map<std::string, Player*> Players;
