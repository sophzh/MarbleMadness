#include "StudentWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_avatar(nullptr), m_numCrystals(0), m_finishedLevel(false), m_bonus(1000)
{}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init()
{
    m_finishedLevel = false;
    m_bonus = 1000;
    ostringstream curLevel;
    curLevel.fill('0');
    int l = GameWorld::getLevel();
    curLevel << "level" << setw(2) << l << ".txt";
    Level lev(assetPath());
    Level::LoadResult result = lev.loadLevel(curLevel.str());
    if (result == Level::load_fail_file_not_found || l == 99)
        return GWSTATUS_PLAYER_WON;
    else if (result == Level::load_fail_bad_format)
        return GWSTATUS_LEVEL_ERROR;
    for (int col = 0; col < VIEW_WIDTH; col++) {
        for (int row = 0; row < VIEW_HEIGHT; row++) {
            Level::MazeEntry item = lev.getContentsOf(col, row);
            switch (item) {
                case Level::empty:
                    break;
                case Level::exit:
                    m_actors.push_back(new Exit(this, col, row));
                    break;
                case Level::player:
                    m_avatar = new Avatar(this, col, row);
                    m_actors.push_back(m_avatar);
                    break;
                case Level::horiz_ragebot:
                    m_actors.push_back(new RageBot(this, col, row, GraphObject::right)); // right
                    break;
                case Level::vert_ragebot:
                    m_actors.push_back(new RageBot(this, col, row, GraphObject::down)); // down
                    break;
                case Level::thiefbot_factory:
                    m_actors.push_back(new ThiefBotFactory(this, col, row, ThiefBotFactory::REGULAR));
                    break;
                case Level::mean_thiefbot_factory:
                    m_actors.push_back(new ThiefBotFactory(this, col, row, ThiefBotFactory::MEAN));
                    break;
                case Level::wall:
                    m_actors.push_back(new Wall(this, col, row));
                    break;
                case Level::marble:
                    m_actors.push_back(new Marble(this, col, row));
                    break;
                case Level::pit:
                    m_actors.push_back(new Pit(this, col, row));
                    break;
                case Level::crystal:
                    m_numCrystals++;
                    m_actors.push_back(new Crystal(this, col, row));
                    break;
                case Level::extra_life:
                    m_actors.push_back(new ExtraLifeGoodie(this, col, row));
                    break;
                case Level::restore_health:
                    m_actors.push_back(new RestoreHealthGoodie(this, col, row));
                    break;
                case Level::ammo:
                    m_actors.push_back(new AmmoGoodie(this, col, row));
                    break;
            }
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    ostringstream score;
    score.fill('0');
    score << setw(7) << getScore();
    
    ostringstream level;
    level.fill('0');
    level << setw(2) << getLevel();
    
    ostringstream lives;
    lives << setw(2) << getLives();
    
    ostringstream health;
    health << setw(3) << (m_avatar->getHP() * 100.0/20.0);
    
    ostringstream ammo;
    ammo << setw(3) << m_avatar->getPeas();
    
    ostringstream bonus;
    bonus << setw(4) << m_bonus;
    
    string s = "Score: " + score.str() + "  Level: " + level.str() + "  Lives: " + lives.str() + "  Health: " + health.str() + "%  Ammo: " + ammo.str() + "  Bonus: " + bonus.str();
    
    setGameStatText(s);
        
    if (m_bonus > 0)
        m_bonus--;

    for (std::list<Actor*>::iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        if ((*a)->isAlive()) {
            (*a)->doSomething();
            if (!m_avatar->isAlive()) {
                decLives();
                m_numCrystals = 0;
                return GWSTATUS_PLAYER_DIED;
            }
        }
        else {
            delete (*a);
            a = m_actors.erase(a);
            a--;
        }
    }
    
    if (m_finishedLevel) {
        increaseScore(2000);
        increaseScore(m_bonus);
        return GWSTATUS_FINISHED_LEVEL;
    }

    
    //exit
    
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    std::list<Actor*>::iterator a = m_actors.begin();
    while (a != m_actors.end()) {
        delete (*a);
        a = m_actors.erase(a);
    }
}

bool StudentWorld::isPlayerOn(int x, int y) const {
    return (m_avatar->getX() == x && m_avatar->getY() == y);
}

bool StudentWorld::canAgentMoveTo(Agent *agent, int col, int row, int dx, int dy) const {
    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        if ((*a)->getX() == col && (*a)->getY() == row) {
            if ((*a)->canCollect(agent)) {
                return true;
            }
            if ((*a)->bePushedTo(agent, col+dx, row+dy)) { //only marbles can be pushed
                (*a)->GraphObject::moveTo(col+dx, row+dy);
                return true;
            }
            if (!(*a)->canAgentColocate(agent)) {
                return false;
            }
        }
    }
    return true;
}

bool StudentWorld::canMarbleMoveTo(int col, int row) const {       //edit later!!
    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        if ((*a)->getX() == col && (*a)->getY() == row) {
            if (!(*a)->allowsMarble()) {
                return false;
            }
        }
    }
    return true;
}

bool StudentWorld::damageSomething(Actor* act, int damageAmt) {
    int col = act->getX();
    int row = act->getY();
    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        if ((*a)->getX() == col && (*a)->getY() == row) {
            if ((*a)->stopsPea()) {
                if ((*a)->isDamageable()) {
                    (*a)->damage(damageAmt);
                }
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::swallowSwallowable(Actor* act) {
    int col = act->getX();
    int row = act->getY();
    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        if ((*a)->getX() == col && (*a)->getY() == row) {
            if ((*a)->isSwallowable()) {
                (*a)->setDead();
                act->setDead();
                return true;
            }
            
        }
    }
    return false;
}

bool StudentWorld::existsClearShotToPlayer(int x, int y, int dx, int dy) const {
    
    int col = m_avatar->getX();
    int row = m_avatar->getY();
    if (x != col && y != row) { //not same column or row
        return false;
    }
    if ((dx == -1 && (row != y || col > x))) { //not same dir - left
        return false;
    }
    if ((dx == 1 && (row != y || col < x))) { //not same dir - right
        return false;
    }
    if ((dy == -1 && (col != x || row > y))) { //not same dir - down
        return false;
    }
    if ((dy == 1 && (col != x || row < y))) { //not same dir - up
        return false;
    }

    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {       //either row or column is the same
        int actx = (*a)->getX();
        int acty = (*a)->getY();
        if (dx == -1 && dy == 0 && acty == row && actx > col && actx < x) { //pea moving left
            if ((*a)->stopsPea()) { return false; }
        }
        if (dx == 1 && dy == 0 && acty == row && actx < col && actx > x) { // pea moving right
            if ((*a)->stopsPea()) { return false; }
        }
        if (dx == 0 && dy == -1 && actx == col && acty > row && acty < y) { // pea moving down
            if ((*a)->stopsPea()) { return false; }
        }
        if (dx == 0 && dy == 1 && actx == col && acty < row && acty > y) { // pea moving up
            if ((*a)->stopsPea()) { return false; }
        }
    }
    return true;
}

Actor* StudentWorld::getColocatedStealable(int x, int y) const {
    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        if ((*a)->getX() == x && (*a)->getY() == y && (*a)->isStealable()) {
            return (*a);
        }
    }
    return nullptr;
}

bool StudentWorld::doFactoryCensus(int x, int y, int distance, int& count) const {
    count = 0;

    for (std::list<Actor*>::const_iterator a = m_actors.begin(); a != m_actors.end(); a++) {
        int col = (*a)->getX();
        int row = (*a)->getY();
        if (col == x && row == y && (*a)->countsInFactoryCensus()) { return false; }
        if ((*a)->countsInFactoryCensus() && col <= x+distance && col >= x-distance && row <= y+distance && row >= y-distance) {
            count++;
        }
    }
    return true;
}


void StudentWorld::addActor(Actor *a) {
    m_actors.push_back(a);
}

bool StudentWorld::anyCrystals() const {
    return (!(m_numCrystals == 0));
}

void StudentWorld::decCrystals() {
    m_numCrystals--;
}

void StudentWorld::increaseAmmo() {
    m_avatar->increaseAmmo(20);
}

void StudentWorld::setLevelFinished() {
    m_finishedLevel = true;
}

int StudentWorld::getTicks() const {
    int ticks = (28 - getLevel()) / 4;
    if (ticks < 3)
        ticks = 3;
    return ticks;
}


