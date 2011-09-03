/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "CommandSpawn.h"
#include <mechanics/GameManager.h>
#include <mechanics/Unit.h>
#include <data/DataManager.h>

CommandSpawn::CommandSpawn(void* player, sf::Uint32 unit_id, sf::Uint32 x_pos, 
                           sf::Uint32 y_pos): ICommand(), unit_id_(unit_id),
                           x_pos_(x_pos), y_pos_(y_pos)
{

}


CommandSpawn::~CommandSpawn()
{

}

void CommandSpawn::execute(GameManager* gm)
{
  Unit *unit = gm->createUnit();
  unit->setPos(x_pos_, y_pos_);
  
  unit->setData(DataManager::Inst()->getUnit(unit_id_));
}