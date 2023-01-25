#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <multithreaded_event_handler.hpp>
#include <world.hpp>
#include <deque>
#include <mutex>

class PhysicsEvent;

class PhysicsEngine {
private:
    std::deque<PhysicsEvent*> event_queue; // not a queue because we need to be able to search for duplicates

    const Uint64 tick_delay = 50;
    SDL_Thread* thread;
    std::mutex mutex;
public:
    bool running;
    PhysicsEngine();
    ~PhysicsEngine();

    void tick();
    void add_event(PhysicsEvent* event);

};

int EngineThread(void* arg);

class PhysicsEvent {
protected:
    World* world;
    PhysicsEngine* engine;
    Multithreaded_Event_Handler* event_handler;
    Uint8 id;
public:
    PhysicsEvent(World* world, PhysicsEngine* engine, Multithreaded_Event_Handler* event_handler, Uint8 id) : world(world), engine(engine), event_handler(event_handler), id(id) {}
    virtual ~PhysicsEvent() {}
    virtual void execute() = 0;
    virtual bool operator==(const PhysicsEvent* other) const = 0; // used to compare events in the queue to avoid duplicates
    Uint8 get_id() const { return id; }
};

class PhysicsEventWater : public PhysicsEvent {
private:
    block_coordonate coord;
    int tick_delay;
public:
    PhysicsEventWater(World* world, PhysicsEngine* engine, Multithreaded_Event_Handler* event_handler, block_coordonate coord, int tick_delay = 10) : PhysicsEvent(world, engine, event_handler, PHYSICS_EVENT_WATER), coord(coord), tick_delay(tick_delay) {}
    void execute();
    bool operator==(const PhysicsEvent* other) const;
    block_coordonate get_coord() const { return coord; }
    friend class PhysicsEngine; // temporary for debugging
};

#endif