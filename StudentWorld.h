#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    bool isPlayerOn(int x, int y) const;
    bool canAgentMoveTo(Agent* agent, int col, int row, int dx, int dy) const;
    bool canMarbleMoveTo(int col, int row) const;
    bool damageSomething(Actor* act, int damageAmt);
    bool swallowSwallowable(Actor* act);
    bool existsClearShotToPlayer(int x, int y, int dx, int dy) const;
    Actor* getColocatedStealable(int x, int y) const;
    bool doFactoryCensus(int x, int y, int distance, int& count) const;
    
    void addActor(Actor* a);
    bool anyCrystals() const;
    void decCrystals();
    void increaseAmmo();
    void setLevelFinished();
    int getTicks() const;
    
private:
    std::list<Actor*> m_actors;
    Avatar* m_avatar;
    int m_numCrystals;
    bool m_finishedLevel;
    int m_bonus;
};

#endif // STUDENTWORLD_H_
