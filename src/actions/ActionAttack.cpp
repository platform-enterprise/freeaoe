#include "ActionAttack.h"
#include "ActionMove.h"
#include "mechanics/UnitManager.h"
#include "core/Constants.h"
#include <genie/dat/Unit.h>

ActionAttack::ActionAttack(const Unit::Ptr &attacker, const Unit::Ptr &target, UnitManager *unitManager) :
    IAction(IAction::Type::Attack, attacker, unitManager),
    m_targetPosition(target->position()),
    m_targetUnit(target)
{
}

ActionAttack::ActionAttack(const Unit::Ptr &attacker, const MapPos &target, UnitManager *unitManager) :
    IAction(IAction::Type::Attack, attacker, unitManager),
    m_targetPosition(target)
{
}

IAction::UnitState ActionAttack::unitState() const
{
    if (m_firing) {
        return IAction::UnitState::Attacking;
    } else {
        return IAction::UnitState::Idle;
    }
}

IAction::UpdateResult ActionAttack::update(Time time)
{
    Unit::Ptr unit = m_unit.lock();
    if (!unit) {
        return IAction::UpdateResult::Completed;
    }

    Unit::Ptr targetUnit;
    if (!m_targetUnit.expired()) {
        targetUnit = m_targetUnit.lock();
        m_targetPosition = targetUnit->position();
    }

    if (targetUnit && targetUnit->healthLeft() <= 0.f) {
        DBG << "Target unit dead";
        return IAction::UpdateResult::Completed;
    }

    // Siege weapon can attack ground
    if (!targetUnit && unit->data()->Class != genie::Unit::SiegeWeapon) {
        DBG << "Target unit gone";
        return IAction::UpdateResult::Completed;
    }

    float timeSinceLastAttack = (time - m_lastAttackTime) * 0.0015;

    if (timeSinceLastAttack < unit->data()->Combat.DisplayedReloadTime) {
        m_firing = true;
    } else {
        m_firing = false;
    }

    const float angleToTarget = atan2(m_targetPosition.y - unit->position().y, m_targetPosition.x - unit->position().x);
    unit->setAngle(angleToTarget - M_PI_2 / 2.);
    const float distance = unit->position().distance(m_targetPosition) / Constants::TILE_SIZE;

    if (distance > unit->data()->Combat.MaxRange || distance < unit->data()->Combat.MinRange) {
        const float angleToTarget = atan2(unit->position().y - m_targetPosition.y, unit->position().x - m_targetPosition.x);
        float targetX = m_targetPosition.x + cos(angleToTarget) * unit->data()->Combat.MinRange * Constants::TILE_SIZE * 1.1;
        float targetY = m_targetPosition.y + sin(angleToTarget) * unit->data()->Combat.MinRange * Constants::TILE_SIZE * 1.1;
        unit->prependAction(ActionMove::moveUnitTo(unit, MapPos(targetX, targetY), m_unitManager->map(), m_unitManager));

        return IAction::UpdateResult::NotUpdated;
    }

    if (timeSinceLastAttack < unit->data()->Combat.ReloadTime) {
        return IAction::UpdateResult::NotUpdated;
    }

    m_lastAttackTime = time;

    return IAction::UpdateResult::Updated;
}
