
//// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <cmath>
#include <cstdlib>

// ACTOR //
Actor::Actor(StudentWorld* sw, int ID, int startX, int startY, int dir, int score, int hp, bool hid) : GraphObject(ID, startX, startY, dir), m_hp(hp), m_world(sw), m_isAlive(true), m_new(true), m_hidden(hid), m_score(score) {
    setVisible(true);
}

Actor::~Actor() {}

bool Actor::canAgentColocate(Agent* a) const {
    if (isHidden())
        return true;
    return false;
}

bool Actor::allowsMarble() const {
    return false;
}

bool Actor::bePushedTo(Agent* a, int x, int y) { return false; }

bool Actor::isDamageable() const { return false; }

void Actor::damage(int amt) {  //check
    setHP(getHP()-amt);
    getWorld()->playSound(impactSound());
    if (getHP() <= 0) {
        setDead();
        getWorld()->playSound(deathSound());
    }
}

bool Actor::stopsPea() const { return true; }

bool Actor::isSwallowable() const { return false; }

bool Actor::isStealable() const { return false; }

bool Actor::canCollect(Agent* a) const { return false; }

bool Actor::countsInFactoryCensus() const { return false; }


bool Actor::isAlive() {
    return m_isAlive;
}

void Actor::setDead() {
    m_isAlive = false;
    setVisible(false);
}

void Actor::decHP(int amt) { m_hp = m_hp - amt; }

int Actor::getHP() const { return m_hp; }
void Actor::setHP(int hp) { m_hp = hp; }

bool Actor::justMade() const { return m_new; }

void Actor::changeNew() { m_new = false; }

void Actor::setStolen(bool status) { m_hidden = status; }

void Actor::unhide() {
    m_hidden = false;
    setVisible(true);
}

void Actor::hide() {
    m_hidden = true;
    setVisible(false);
}

bool Actor::isHidden() const { return m_hidden; }

int Actor::returnScore() const { return m_score; }

int Actor::deathSound() const { return SOUND_NONE; }
int Actor::impactSound() const { return SOUND_NONE; }


StudentWorld* Actor::getWorld() const { return m_world; }


// AGENT //
Agent::Agent(StudentWorld* sw, int ID, int startX, int startY, int dir, int score, int hp) : Actor(sw, ID, startX, startY, dir, score, hp) {}

Agent::~Agent() {}

bool Agent::isDamageable() const { return true; }

bool Agent::canPushMarbles() const { return false; }


// AVATAR //
Avatar::Avatar(StudentWorld* sw, int startX, int startY) : Agent(sw, IID_PLAYER, startX, startY, right, 0, 20), m_peas(20) {}

void Avatar::doSomething() {
    if (!Actor::isAlive()) {
        return;
    }
    int ch;
    if (getWorld()->getKey(ch)) {
     // user hit a key this tick!
        int x = this->GraphObject::getX();
        int y = this->GraphObject::getY();
        switch (ch)
        {
            case KEY_PRESS_LEFT:
                GraphObject::setDirection(left);
                if (getWorld()->canAgentMoveTo(this, x-1, y, -1, 0)) {
                    this->GraphObject::moveTo(getX()-1, getY());
                }
                break;
            case KEY_PRESS_RIGHT:
                GraphObject::setDirection(right);
                if (getWorld()->canAgentMoveTo(this, x+1, y, 1, 0)) {
                    this->GraphObject::moveTo(getX()+1, getY());
                }
                break;
            case KEY_PRESS_DOWN:
                GraphObject::setDirection(down);
                if (getWorld()->canAgentMoveTo(this, x, y-1, 0, -1)) {
                    this->GraphObject::moveTo(getX(), getY()-1);
                }
                break;
            case KEY_PRESS_UP:
                GraphObject::setDirection(up);
                if (getWorld()->canAgentMoveTo(this, x, y+1, 0, 1)) {
                    this->GraphObject::moveTo(getX(), getY()+1);
                }
                break;
            case KEY_PRESS_SPACE: {
                if (getPeas() > 0) {
                    int dx = 0; int dy = 0;
                    if (getDirection() == left) { dx = -1; }
                    if (getDirection() == right) { dx = 1; }
                    if (getDirection() == down) { dy = -1; }
                    if (getDirection() == up) { dy = 1; }
                    getWorld()->addActor(new Pea(getWorld(), x+dx, y+dy, getDirection(), this));
                    getWorld()->playSound(SOUND_PLAYER_FIRE);
                    useAmmo();
                }
                break;
            case KEY_PRESS_ESCAPE:
                setDead();
                getWorld()->playSound(SOUND_PLAYER_DIE);
                break;
            }
            default:
                break;
        }
    }
}


bool Avatar::canPushMarbles() const { return true; }

int Avatar::deathSound() const { return SOUND_PLAYER_DIE; }
int Avatar::impactSound() const { return SOUND_PLAYER_IMPACT; }

int Avatar::getPeas() const { return m_peas; }

void Avatar::increaseAmmo(int amt) {
    m_peas += amt;
}

void Avatar::useAmmo() {
    m_peas--;
}

// ROBOT //
Robot::Robot(StudentWorld* sw, int ID, int startX, int startY, int dir, int score, int hp) : Agent(sw, ID, startX, startY, dir, score, hp) {}

void Robot::doSomething() {
    incTicks();
    if (!isAlive() || !moveThisRound()) {
        return;
    }
    
    int dx = 0; int dy = 0;
    if (getDirection() == left) { dx = -1; }
    if (getDirection() == right) { dx = 1; }
    if (getDirection() == down) { dy = -1; }
    if (getDirection() == up) { dy = 1; }
    
    if (isShootingRobot()) {
        if (getWorld()->existsClearShotToPlayer(getX(), getY(), dx, dy)) {
            getWorld()->addActor(new Pea(getWorld(), getX()+dx, getY()+dy, getDirection(), this));
            getWorld()->playSound(SOUND_ENEMY_FIRE);
            return;
        }
    }
    
    if (doesSteal()) {
        bool success = steal();
        if (success) {
            return;
        }
    }
    
    doThing(dx, dy);
}

bool Robot::isDamageable() const { return true; }

bool Robot::isShootingRobot() const { return false; }

bool Robot::doesSteal() const { return false; }

int Robot::deathSound() const { return SOUND_ROBOT_DIE; }
int Robot::impactSound() const { return SOUND_ROBOT_IMPACT; }

bool Robot::moveThisRound() const {
    
    return (m_tickcount % (getWorld()->getTicks()) == 0);
}

void Robot::incTicks() { m_tickcount++; }

bool Robot::steal() { return false; }

// RAGEBOT //
RageBot::RageBot(StudentWorld* sw, int startX, int startY, int dir) : Robot(sw, IID_RAGEBOT, startX, startY, dir, 100, 10) {}

void RageBot::doThing(int dx, int dy) {
    
    if (getWorld()->canAgentMoveTo(this, getX()+dx, getY()+dy, dx, dy)) {
        moveTo(getX()+dx, getY()+dy);
    }
    else {
        setDirection((getDirection()+180) % 360);
    }
    
}

bool RageBot::isShootingRobot() const { return true; }

// THIEFBOT //
ThiefBot::ThiefBot(StudentWorld* sw, int ID, int startX, int startY, int score, int hp) : Robot(sw, ID, startX, startY, right, score, hp), m_distance(0), m_stolenGoods(nullptr) {}

void ThiefBot::doThing(int dx, int dy) {
    if (m_distance <= 0 || !getWorld()->canAgentMoveTo(this, getX()+dx, getY()+dy, dx, dy)) {
        m_distance = rand() % 6 + 1;
        int direction = ((rand() % 4) * 90);
        
        int count = 1;
        if (direction == left) { dx = -1; dy = 0; }
        if (direction == right) { dx = 1; dy = 0; }
        if (direction == down) { dy = -1; dx = 0; }
        if (direction == up) { dy = 1; dx = 0; }
        while (!getWorld()->canAgentMoveTo(this, getX()+dx, getY()+dy, dx, dy) && count <= 4) {
            direction  = (direction + 90) % 360;
            if (direction == left) { dx = -1; dy = 0; }
            if (direction == right) { dx = 1; dy = 0; }
            if (direction == down) { dy = -1; dx = 0; }
            if (direction == up) { dy = 1; dx = 0; }
            count++;
        }
        setDirection(direction);
    }
    if (getWorld()->canAgentMoveTo(this, getX()+dx, getY()+dy, dx, dy)) {
        moveTo(getX()+dx, getY()+dy);
        if (m_stolenGoods != nullptr) {
            m_stolenGoods->moveTo(getX(), getY());
        }
        m_distance--;
    }
    return;
}

bool ThiefBot::countsInFactoryCensus() const { return true; }

bool ThiefBot::doesSteal() const { return true; }

bool ThiefBot::steal() {
    Actor* temp = getWorld()->getColocatedStealable(getX(), getY());
    if (temp == nullptr) {
        return false;
    }
    int chance = rand() % 10;
    if (chance % 10 != 0) { return false; }
    getWorld()->playSound(SOUND_ROBOT_MUNCH);
    m_stolenGoods = temp;
    m_stolenGoods->setStolen(true);
    return true;
}

void ThiefBot::damage(int amt) {  //check
    setHP(getHP()-amt);
    getWorld()->playSound(impactSound());
    if (getHP() <= 0) {
        setDead();
        getWorld()->playSound(deathSound());
        if (m_stolenGoods != nullptr) {
            m_stolenGoods->setStolen(false);
            m_stolenGoods = nullptr;
        }
    }
}

// REGULAR THIEFBOT //
RegularThiefBot::RegularThiefBot(StudentWorld* sw, int startX, int startY) : ThiefBot(sw, IID_THIEFBOT, startX, startY, 10, 5) {}

// MEAN THIEFBOT //
MeanThiefBot::MeanThiefBot(StudentWorld* sw, int startX, int startY) : ThiefBot(sw, IID_MEAN_THIEFBOT, startX, startY, 20, 8) {}

bool MeanThiefBot::isShootingRobot() const { return true; }

// THIEFBOT FACTORY //
ThiefBotFactory::ThiefBotFactory(StudentWorld* sw, int startX, int startY, ProductType type) : Actor(sw, IID_ROBOT_FACTORY, startX, startY), m_tbtype(type) {}

void ThiefBotFactory::doSomething() {
    int count = 0;
    if (getWorld()->doFactoryCensus(getX(), getY(), 3, count)) {
        int temp = rand() % 50;
        if (count < 3 && (temp % 50 == 0)) {
            getWorld()->playSound(SOUND_ROBOT_BORN);
            if (m_tbtype == REGULAR) {
                getWorld()->addActor(new RegularThiefBot(getWorld(), getX(), getY()));
            }
            else if (m_tbtype == MEAN) {
                getWorld()->addActor(new MeanThiefBot(getWorld(), getX(), getY()));
            }
        }
    }
}

// WALL //
Wall::Wall(StudentWorld* sw, int startX, int startY) : Actor(sw, IID_WALL, startX, startY) {}

void Wall::doSomething() { return; }

// MARBLE //
Marble::Marble(StudentWorld* sw, int startX, int startY) : Actor(sw, IID_MARBLE, startX, startY, none, 0, 10) {}

void Marble::doSomething() { return; }

bool Marble::isDamageable() const { return true; }

bool Marble::isSwallowable() const { return true; }

bool Marble::stopsPea() const { return true; }

bool Marble::bePushedTo(Agent* a, int x, int y) { //work on this
    if (!a->canPushMarbles()) {
        return false;
    }
    if (Actor::getWorld()->canMarbleMoveTo(x, y)) {
        return true;
    }
    return false;
}

// PEA //

Pea::Pea(StudentWorld* sw, int startX, int startY, int dir, Agent* ag) : Actor(sw, IID_PEA, startX, startY, dir) {}

void Pea::doSomething() {
    if (!isAlive()) {
        return;
    }
    if (justMade()) {
        changeNew();
        return;
    }
    if (getWorld()->damageSomething(this, 2)) {
        setDead();
        return;
    }
    int dx = 0; int dy = 0;
    if (getDirection() == left) { dx = -1; }
    if (getDirection() == right) { dx = 1; }
    if (getDirection() == down) { dy = -1; }
    if (getDirection() == up) { dy = 1; }
    moveTo(getX()+dx, getY()+dy);
    if (getWorld()->damageSomething(this, 2)) {
        setDead();
        return;
    }
}

bool Pea::canAgentColocate(Agent* a) const { return true; }

bool Pea::allowsMarble() const { return true; }

bool Pea::stopsPea() const { return false; }

// PIT //
Pit::Pit(StudentWorld* sw, int startX, int startY) : Actor(sw, IID_PIT, startX, startY, none) {}

void Pit::doSomething() {
    if (!isAlive()) {
        return;
    }
    getWorld()->swallowSwallowable(this);
}

bool Pit::allowsMarble() const { return true; }

bool Pit::stopsPea() const { return false; }

// EXIT //
Exit::Exit(StudentWorld* sw, int startX, int startY) : Actor(sw, IID_EXIT, startX, startY, none, 0, 999, true) {
    setVisible(false);
    
}
void Exit::doSomething() {
    if (isHidden() && getWorld()->anyCrystals()) {
        return;
    }
    if (isHidden() && !(getWorld()->anyCrystals())) {
        unhide();
        getWorld()->playSound(SOUND_REVEAL_EXIT);
    }
    if (!isHidden() && getWorld()->isPlayerOn(getX(), getY())) {
        getWorld()->playSound(SOUND_FINISHED_LEVEL);
        getWorld()->setLevelFinished();
    }
}

bool Exit::stopsPea() const { return false; }

bool Exit::canAgentColocate(Agent* a) const {
    if (isHidden()) {
        return true;
    }
    if (a->canPushMarbles()) {
        return true;
    }
    return false;
}


// PICKUPABLE ITEM //
PickupableItem::PickupableItem(StudentWorld* sw, int ID, int startX, int startY, int score) : Actor(sw, ID, startX, startY, none, score) {}

bool PickupableItem::stopsPea() const { return false; }

bool PickupableItem::canCollect(Agent* a) const {
    if (!a->canPushMarbles()) {
        return false;
    }
    return true;
}

void PickupableItem::collect() {
    setDead();
    getWorld()->increaseScore(returnScore());
    getWorld()->playSound(SOUND_GOT_GOODIE);
}

bool PickupableItem::canAgentColocate(Agent* a) const {
    return true;
}

// CRYSTAL //
Crystal::Crystal(StudentWorld* sw, int startX, int startY) : PickupableItem(sw, IID_CRYSTAL, startX, startY, 50) {}

void Crystal::doSomething() {
    if (!isAlive()) {
        return;
    }
    if (getWorld()->isPlayerOn(getX(), getY())) {
        collect();
        getWorld()->decCrystals();
    }
    
}

// GOODIES //

Goodie::Goodie(StudentWorld* sw, int ID, int startX, int startY, int score) : PickupableItem(sw, ID, startX, startY, score), m_stolen(false) {}

void Goodie::doSomething() {
    if (!isAlive() || m_stolen) {
        return;
    }
    if (getWorld()->isPlayerOn(getX(), getY())) {
        collect();
        addThing();
    }
}

bool Goodie::isStealable() const {
    if (m_stolen) {
        return false;
    }
    return true;
}

void Goodie::setStolen(bool status) {
    m_stolen = status;
    if (status) { hide(); }
    else { unhide(); }
}

// EXTRA LIFE //
ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* sw, int startX, int startY) : Goodie(sw, IID_EXTRA_LIFE, startX, startY, 1000) {}

void ExtraLifeGoodie::addThing() {
    getWorld()->incLives();
}

// RESTORE HEALTH //
RestoreHealthGoodie::RestoreHealthGoodie(StudentWorld* sw, int startX, int startY) : Goodie(sw, IID_RESTORE_HEALTH, startX, startY, 500) {}

void RestoreHealthGoodie::addThing() {
    setHP(20);
}

// AMMO //

AmmoGoodie::AmmoGoodie(StudentWorld* sw, int startX, int startY) : Goodie(sw, IID_AMMO, startX, startY, 100) {}

void AmmoGoodie::addThing() {
    getWorld()->increaseAmmo();
}
