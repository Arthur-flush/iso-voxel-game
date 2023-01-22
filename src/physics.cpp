#include <thread>
#include <chrono>
#include <algorithm>

#include "physics.hpp"

void PhysicsEventWater::execute() {
    if (tick_delay > 0) {
        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, coord, tick_delay - 1);
        engine->add_event(event);
        return;
    }

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue; // skip self
            if (x != 0 && y != 0) continue; // skip diagonals

            block_coordonate neighbour = coord + coord3D({x, y, 0});
            

            if ( 
                // check if neighbour is in world
                neighbour.chunk.x >= 0 && neighbour.chunk.x < world->max_chunk_coord.x &&
                neighbour.chunk.y >= 0 && neighbour.chunk.y < world->max_chunk_coord.y &&
                

                // check if neighbour is empty
                world->chunks[neighbour.chunk.x][neighbour.chunk.y][neighbour.chunk.z].blocks[neighbour.x][neighbour.y][neighbour.z].id == BLOCK_EMPTY
                ) {
                world_coordonate wcoord = world->convert_coord(neighbour);
                // std::cout << "water spread to " << wcoord.x << ", " << wcoord.y << ", " << wcoord.z << std::endl;
                event_handler->add_event(GAME_EVENT_SINGLE_BLOCK_MOD, wcoord, BLOCK_WATER);
                PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, neighbour);
                engine->add_event(event);
            }
            
        }
    }

    // check {0, 0, -1}
    block_coordonate neighbour = coord + coord3D({0, 0, -1});
    if (
        neighbour.chunk.z >= 0 && neighbour.chunk.z < world->max_chunk_coord.z &&
        world->chunks[neighbour.chunk.x][neighbour.chunk.y][neighbour.chunk.z].blocks[neighbour.x][neighbour.y][neighbour.z].id == BLOCK_EMPTY
        ) {
        world_coordonate wcoord = world->convert_coord(neighbour);
        event_handler->add_event(GAME_EVENT_SINGLE_BLOCK_MOD, wcoord, BLOCK_WATER);
        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, neighbour);
        engine->add_event(event);
        }
}

bool PhysicsEventWater::operator==(const PhysicsEvent* event) const {
    if (event->get_id() != PHYSICS_EVENT_WATER) return false;
    const PhysicsEventWater* event_cast = static_cast<const PhysicsEventWater*>(event);
    return coord == event_cast->get_coord();
}



PhysicsEngine::~PhysicsEngine() {
    while (!event_queue.empty()) {
        delete event_queue.front();
        event_queue.pop_front();
    }
}

void PhysicsEngine::tick() {
    Uint64 start = Get_time_ms();
   
    mutex.lock();
    // std::cout << "mutex lock in tick" << std::endl;

    std::deque <PhysicsEvent*> event_queue_copy;

    for (PhysicsEvent* event : event_queue) {
        event_queue_copy.push_back(event);
    }

    event_queue.clear();


    // std::cout << "mutex unlock in tick" << std::endl;
    mutex.unlock();

    
    while(!event_queue_copy.empty()) {
        event_queue_copy.front()->execute();
        delete event_queue_copy.front();
        event_queue_copy.pop_front();
    }

    Uint64 end = Get_time_ms();
    Uint64 delta = end - start;

    if (delta < tick_delay) {
        std::cout << "sleeping for " << tick_delay - delta << " ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_delay - delta));
    }
    
}

void PhysicsEngine::add_event(PhysicsEvent* event) {
    mutex.lock();

    // int size = event_queue.size();
    // if (size == 8) {
    //     PhysicsEventWater* new_event_cast = static_cast<PhysicsEventWater*>(event);
    //     std::cout << "new event: " << new_event_cast->coord.x << ", " << new_event_cast->coord.y << ", " << new_event_cast->coord.z << std::endl;
        
    //     std::cout << "current events:" << std::endl;
    //     int i = 0;
    //     for (auto event_in_queue : event_queue) {
    //         PhysicsEventWater* event_cast = static_cast<PhysicsEventWater*>(event_in_queue);
    //         std::cout << i << ": " << event_cast->coord.x << ", " << event_cast->coord.y << ", " << event_cast->coord.z << std::endl;
    //         if (event_cast->coord == new_event_cast->coord) {
    //             std::cout << "same event" << std::endl;
    //             if (less_than(*event_cast, event)) {
    //                 std::cout << "event_in_queue < event" << std::endl;
    //             }
    //             else if (less_than(*new_event_cast, event_in_queue)) {
    //                 std::cout << "event < event_in_queue" << std::endl;
    //             }
    //             else {
    //                 std::cout << "event == event_in_queue" << std::endl;
    //             }
    //         }
            
    //         i++;
    //     }
    // }

    // check if event is already in queue
    auto it = std::find_if(event_queue.begin(), event_queue.end(), [event](PhysicsEvent* event_in_queue) {
        return *event_in_queue == event;
    });

    if (it != event_queue.end()) {
        // event is already in queue
        // std::cout << "event already in queue" << std::endl;
        delete event;
    }
    else {
        // event is not in queue
        // std::cout << "inserted event" << std::endl;
        event_queue.push_back(event);
    }
    mutex.unlock();
}

int EngineThread(void* arg) {
    PhysicsEngine* engine = (PhysicsEngine*)arg;
    while (engine->running) {
        engine->tick();
    }
    return 0;
}

PhysicsEngine::PhysicsEngine() {
    running = true;
    thread = SDL_CreateThread(EngineThread, "PhysicsEngine", this);
}