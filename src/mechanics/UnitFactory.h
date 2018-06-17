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

#ifndef UNITFACTORY_H
#define UNITFACTORY_H

#include "../core/Entity.h"
#include "mechanics/Player.h"

class UnitFactory
{
public:
    static UnitFactory &Inst();

    Unit::Ptr createUnit(int ID, const MapPos &position, Player::Ptr owner);

private:
    UnitFactory();
    virtual ~UnitFactory();
};

#endif // UNITFACTORY_H
