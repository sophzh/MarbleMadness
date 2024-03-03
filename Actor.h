#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Agent;

class Actor: public GraphObject {
public:
    Actor(StudentWorld* sw, int ID, int startX, int startY, int dir = none, int score = 0, int hp = 999, bool hid = false);
    virtual ~Actor();
    virtual void doSomething() = 0;
    
    virtual bool canAgentColocate(Agent* a) const; //false
    virtual bool allowsMarble() const; //false
    virtual bool isDamageable() const; //false
    virtual void damage(int amt);
    virtual bool stopsPea() const; //true
    virtual bool isSwallowable() const; //false
    virtual bool isStealable() const; //false
    virtual bool canCollect(Agent* a) const; //false
    virtual bool countsInFactoryCensus() const; //false
    
    virtual bool bePushedTo(Agent* a, int x, int y); //false
    
    bool isAlive();
    void setDead();
    
    void decHP(int amt);
    int getHP() const;
    void setHP(int hp);
    bool justMade() const;
    void changeNew();
    virtual void setStolen(bool status);
    virtual void unhide();
    virtual void hide();
    bool isHidden() const;
    int returnScore() const;
    virtual int deathSound() const;
    virtual int impactSound() const;
    
    StudentWorld* getWorld() const;
    
private:
    StudentWorld* m_world;
    int m_hp;
    bool m_isAlive;
    bool m_new;
    bool m_hidden;
    int m_score;
};


class Agent: public Actor {
public:
    Agent(StudentWorld* sw, int ID, int startX, int startY, int dir, int score, int hp);
    virtual ~Agent();
    virtual void doSomething() = 0;
    
    virtual bool isDamageable() const;
    
    virtual bool canPushMarbles() const;

};

class Avatar: public Agent {
public:
    Avatar(StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
    
    virtual bool canPushMarbles() const; //false
    virtual int deathSound() const;
    virtual int impactSound() const;
    
    int getPeas() const;
    void increaseAmmo(int amt);
    void useAmmo();
    
private:
    int m_peas;
};

class Robot : public Agent {
public:
    Robot(StudentWorld* sw, int ID, int startX, int startY, int dir, int score, int hp);
    virtual void doSomething();
    virtual bool isDamageable() const; //true

          // Does this robot shoot?
    virtual bool isShootingRobot() const; //false
    virtual bool doesSteal() const; //false
    virtual int deathSound() const;
    virtual int impactSound() const;
    bool moveThisRound() const;
    void incTicks();
    virtual bool steal(); //nothing
    virtual void doThing(int dx, int dy) = 0;
    
private:
    int m_tickcount;
};

class RageBot : public Robot {
public:
    RageBot(StudentWorld* sw, int startX, int startY, int dir);
    virtual void doThing(int dx, int dy);
    virtual bool isShootingRobot() const;
};

class ThiefBot : public Robot {
public:
    ThiefBot(StudentWorld* sw, int ID, int startX, int startY, int score, int hp);
    virtual void doThing(int dx, int dy);
    virtual bool countsInFactoryCensus() const;
    virtual bool doesSteal() const;
    
    virtual bool steal();
    
    virtual void damage(int amt);
        
private:
    int m_distance;
    Actor* m_stolenGoods;
};

class RegularThiefBot : public ThiefBot {
public:
    RegularThiefBot(StudentWorld* sw, int startX, int startY);
};

class MeanThiefBot : public ThiefBot {
public:
    MeanThiefBot(StudentWorld* sw, int startX, int startY);
    virtual bool isShootingRobot() const;
};

class ThiefBotFactory : public Actor {
public:
    enum ProductType { REGULAR, MEAN };

    ThiefBotFactory(StudentWorld* sw, int startX, int startY, ProductType type);
    virtual void doSomething();
    
private:
    ProductType m_tbtype;
};


class Wall : public Actor {
public:
    Wall(StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
};

class Marble : public Actor {
public:
    Marble(StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
    
    virtual bool isDamageable() const;
    virtual bool isSwallowable() const;
    virtual bool stopsPea() const;
    
    virtual bool bePushedTo(Agent* a, int x, int y);
    
};

class Pea : public Actor {
public:
    Pea(StudentWorld* sw, int startX, int startY, int dir, Agent* ag);
    virtual void doSomething();
    virtual bool canAgentColocate(Agent* a) const;
    virtual bool allowsMarble() const;
    virtual bool stopsPea() const;
};

class Pit : public Actor {
public:
    Pit(StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
    virtual bool allowsMarble() const;
    virtual bool stopsPea() const;
};

class Exit : public Actor {
public:
    Exit(StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
    virtual bool stopsPea() const;
    
    bool canAgentColocate(Agent* a) const;
};

class PickupableItem : public Actor {
public:
    PickupableItem(StudentWorld* sw, int ID, int startX, int startY, int score);
    virtual void doSomething() = 0;
    virtual bool stopsPea() const;
    virtual bool canCollect(Agent* a) const;
    virtual void collect();
    virtual bool canAgentColocate(Agent* a) const;

};

class Crystal : public PickupableItem {
public:
    Crystal(StudentWorld* sw, int startX, int startY);
    virtual void doSomething();
};

class Goodie : public PickupableItem {
public:
    Goodie(StudentWorld* sw, int ID, int startX, int startY, int score);
    virtual void doSomething();
    virtual bool isStealable() const;

    virtual void setStolen(bool status);
    virtual void addThing() = 0;
private:
    bool m_stolen;
};

class ExtraLifeGoodie : public Goodie {
public:
    ExtraLifeGoodie(StudentWorld* sw, int startX, int startY);
    virtual void addThing();
};

class RestoreHealthGoodie : public Goodie {
public:
    RestoreHealthGoodie(StudentWorld* sw, int startX, int startY);
    virtual void addThing();
};

class AmmoGoodie : public Goodie {
public:
    AmmoGoodie(StudentWorld* sw, int startX, int startY);
    virtual void addThing();
};

#endif // ACTOR_H_

