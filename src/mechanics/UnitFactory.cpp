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

#include "UnitFactory.h"

#include <genie/dat/Unit.h>
#include <genie/dat/ResourceUsage.h>
#include <genie/dat/unit/Action.h>
#include <genie/dat/unit/Building.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "Building.h"
#include "Civilization.h"
#include "Farm.h"
#include "Player.h"
#include "UnitManager.h"
#include "actions/ActionFly.h"
#include "actions/IAction.h"
#include "core/Logger.h"
#include "core/ResourceMap.h"
#include "core/Utility.h"
#include "render/GraphicRender.h"
#include "resource/DataManager.h"

class UnitManager;


UnitFactory &UnitFactory::Inst()
{
    static UnitFactory inst;
    return inst;
}

Unit::Ptr UnitFactory::duplicateUnit(const Unit::Ptr &other)
{
    if (!other) {
        WARN << "can't duplicate null unit";
        return nullptr;
    }
    Player::Ptr owner = other->player().lock();
    if (!owner) {
        WARN << "can't duplicate unit without owner";
        return nullptr;
    }

    return createUnit(other->data()->ID, owner, other->unitManager());
}

void UnitFactory::handleDefaultAction(const Unit::Ptr &unit, const Task &task)
{
    switch(task.data->ActionType) {
    case genie::ActionType::Fly: {
        MapPos flyingPosition = unit->position();

        // The below comment will stay just to illustrate what one of my professors once told me
        // about commenting in source code; don't, comments always get outdated
        // and confuse more than clarify.

        // Castles are 4 high, so set 5.5 just to be safe that we fly above everything
        if (flyingPosition.z < 10) {
            flyingPosition.z = 10;
            unit->setPosition(flyingPosition);
        }
        unit->actions.queueAction(std::make_shared<ActionFly>(unit, task));
        break;
    }

        //TODO
    case genie::ActionType::Graze:
        break;
    case genie::ActionType::GetAutoConverted:
        unit->actions.autoConvert =true;
        break;
    default:
        WARN << "unhandled default action" << task.data->actionTypeName() << "for" << unit->debugName;
    }

}

static bool checkForAutoConvert(const Unit::Ptr &unit)
{
    if (unit->playerId() != UnitManager::GaiaID) {
        return false;
    }

    // No idea if there's a better way...
    switch(unit->data()->Class) {
    case genie::Unit::Archer:
    case genie::Unit::Artifact:
    case genie::Unit::TradeBoat:
    case genie::Unit::BuildingClass:
    case genie::Unit::Civilian:
    case genie::Unit::Infantry:
    case genie::Unit::Cavalry:
    case genie::Unit::SiegeWeapon:
    case genie::Unit::Healer:
    case genie::Unit::Monk:
    case genie::Unit::TradeCart:
    case genie::Unit::TransportBoat:
    case genie::Unit::FishingBoat:
    case genie::Unit::Warship:
    case genie::Unit::Conquistador:
    case genie::Unit::WarElephant:
    case genie::Unit::Hero:
    case genie::Unit::ElephantArcher:
    case genie::Unit::Wall:
    case genie::Unit::Phalanx:
    case genie::Unit::Flag:
    case genie::Unit::Petard:
    case genie::Unit::CavalryArcher:
    case genie::Unit::MonkWithRelic:
    case genie::Unit::HandCannoneer:
    case genie::Unit::TwoHandedSwordsman:
    case genie::Unit::Pikeman:
    case genie::Unit::Scout:
    case genie::Unit::Farm:
    case genie::Unit::Spearman:
    case genie::Unit::PackedUnit:
    case genie::Unit::Tower:
    case genie::Unit::BoardingBoat:
    case genie::Unit::UnpackedSiegeUnit:
    case genie::Unit::Ballista:
    case genie::Unit::Raider:
    case genie::Unit::CavalryRaider:
    case genie::Unit::King:
    case genie::Unit::MiscBuilding:
        return true;

    case genie::Unit::OceanFish:
    case genie::Unit::BerryBush:
    case genie::Unit::StoneMine:
    case genie::Unit::PreyAnimal:
    case genie::Unit::PredatorAnimal:
    case genie::Unit::Miscellaneous:
    case genie::Unit::TerrainClass:
    case genie::Unit::Tree:
    case genie::Unit::TreeStump:
    case genie::Unit::DomesticAnimal:
    case genie::Unit::DeepSeaFish:
    case genie::Unit::GoldMine:
    case genie::Unit::ShoreFish:
    case genie::Unit::Cliff:
    case genie::Unit::Doppelganger:
    case genie::Unit::Bird:
    case genie::Unit::Gate:
    case genie::Unit::SalvagePile:
    case genie::Unit::ResourcePile:
    case genie::Unit::Relic:
    case genie::Unit::OreMine:
    case genie::Unit::Livestock:
    case genie::Unit::ControlledAnimal:
        return false;
    default:
        WARN << "Unknown class" << unit->data()->Class;
        break;
    }
    return unit->data()->Type < genie::Unit::CombatantType;
}

Unit::Ptr UnitFactory::createUnit(const int ID, const Player::Ptr &owner, UnitManager &unitManager)
{
    const genie::Unit &gunit = owner->civilization.unitData(ID);
    owner->applyResearch(gunit.Building.TechID);

    Unit::Ptr unit;
    if (ID == Unit::Farm) { // Farms are very special (shortbus special), so better to just use a special class
        unit = std::make_shared<Farm>(gunit, owner, unitManager);
    } else if (gunit.Type == genie::Unit::BuildingType) {
        unit = std::make_shared<Building>(gunit, owner, unitManager);
    } else {
        unit = std::make_shared<Unit>(gunit, owner, unitManager);
    }

    if (!unit->renderer().sprite()) {
        WARN << "Failed to load graphics for" << unit->debugName;
        return nullptr;
    }


    for (const genie::Unit::ResourceStorage &res : gunit.ResourceStorages) {
        if (res.Type == -1) {
            continue;
        }

        unit->resources[genie::ResourceType(res.Type)] = res.Amount;
    }

    if (gunit.Class == genie::Unit::Farm) {
        unit->resources[genie::ResourceType::FoodStorage] = owner->civilization.startingResource(genie::ResourceType::FarmFoodAmount);

    }

    if (gunit.Type >= genie::Unit::BuildingType) {
        if (gunit.Building.StackUnitID >= 0) {
            const genie::Unit &stackData = owner->civilization.unitData(gunit.Building.StackUnitID);
            owner->applyResearch(gunit.Building.TechID);

            Unit::Annex annex;
            annex.unit = std::make_shared<Unit>(stackData, owner, unitManager);
            unit->annexes.push_back(annex);
        }

        for (const genie::unit::BuildingAnnex &annexData : gunit.Building.Annexes) {
            if (annexData.UnitID < 0) {
                continue;
            }
            const genie::Unit &stackGUnit = owner->civilization.unitData(annexData.UnitID);
            owner->applyResearch(stackGUnit.Building.TechID);

            Unit::Annex annex;
            annex.offset = MapPos(annexData.Misplacement.first * -48, annexData.Misplacement.second * -48);
            annex.unit = std::make_shared<Unit>(stackGUnit, owner, unitManager);
            unit->annexes.push_back(annex);
        }

        if (!unit->annexes.empty()) {
            std::reverse(unit->annexes.begin(), unit->annexes.end());
        }
    }

    for (const Task &task : unit->actions.availableActions()) {
        if (task.taskId == gunit.Action.DefaultTaskID) {
            handleDefaultAction(unit, task);
        } else if (task.data->IsDefault) {
            handleDefaultAction(unit, task);
            continue;
        }
    }

    if (!unit->actions.autoConvert) {
        unit->actions.autoConvert = checkForAutoConvert(unit);
    }

    return unit;
}

DecayingEntity::Ptr UnitFactory::createCorpseFor(const Unit::Ptr &unit)
{
    if (IS_UNLIKELY(!unit)) {
        WARN << "can't create corpse for null unit";
        return nullptr;
    }

    Player::Ptr owner = unit->player().lock();
    if (!owner) {
        WARN << "no owner for corpse";
        return nullptr;
    }

    if (unit->data()->DeadUnitID == -1) {
        return nullptr;
    }

    const genie::Unit &corpseData = owner->civilization.unitData(unit->data()->DeadUnitID);
    float decayTime = corpseData.ResourceDecay * corpseData.ResourceCapacity;

    for (const genie::Unit::ResourceStorage &r : corpseData.ResourceStorages) {
        if (r.Type == int(genie::ResourceType::CorpseDecayTime)) {
            decayTime = r.Amount;
            break;
        }
    }

    // I don't think this is really correct, but it works better
    if (corpseData.ResourceDecay == -1 || (corpseData.ResourceDecay != 0 && corpseData.ResourceCapacity == 0)) {
        DBG << "decaying forever";
        decayTime = std::numeric_limits<float>::infinity();
    }
    DecayingEntity::Ptr corpse = std::make_shared<DecayingEntity>(
                corpseData.StandingGraphic.first,
                decayTime,
                Size(corpseData.Size)
                );
    corpse->renderer().setPlayerColor(owner->playerColor);
    corpse->setMap(unit->map());
    corpse->setPosition(unit->position());
    corpse->renderer().setAngle(unit->angle());

    return corpse;

}

std::shared_ptr<DopplegangerEntity> UnitFactory::createDopplegangerFor(const std::shared_ptr<Unit> &unit)
{
    REQUIRE(unit, return nullptr);

    DopplegangerEntity::Ptr doppleganger = std::make_shared<DopplegangerEntity>(unit);
    DBG << "Created doppleganger";
    doppleganger->setMap(unit->map());
    doppleganger->setPosition(unit->position());

    return doppleganger;

}

std::shared_ptr<DecayingEntity> UnitFactory::createCorpseFor(const std::shared_ptr<DopplegangerEntity> &doppleganger)
{
    REQUIRE(doppleganger, return nullptr);

    const genie::Unit *data = doppleganger->originalUnitData;
    if (data->DeadUnitID == -1) {
        return nullptr;
    }

    float decayTime = data->ResourceDecay * data->ResourceCapacity;

    for (const genie::Unit::ResourceStorage &r : data->ResourceStorages) {
        if (r.Type == int(genie::ResourceType::CorpseDecayTime)) {
            decayTime = r.Amount;
            break;
        }
    }

    // I don't think this is really correct, but it works better
    if (data->ResourceDecay == -1 || (data->ResourceDecay != 0 && data->ResourceCapacity == 0)) {
        DBG << "decaying forever";
        decayTime = std::numeric_limits<float>::infinity();
    }

    DecayingEntity::Ptr corpse = std::make_shared<DecayingEntity>(
                data->StandingGraphic.first,
                decayTime,
                Size(data->Size)
                );
    corpse->renderer().setPlayerColor(doppleganger->renderer().playerColor());
    corpse->setMap(doppleganger->map());
    corpse->setPosition(doppleganger->position());
    corpse->renderer().setAngle(doppleganger->renderer().angle());

    return corpse;
}

