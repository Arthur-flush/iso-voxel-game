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

    // engine->world_mutex.lock();
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue; // skip self
            if (x != 0 && y != 0) continue; // skip diagonals

            block_coordonate neighbour = coord + coord3D({x, y, 0});

            int support_count = 0;
            bool has_support = false;
            for (int a = 0; a <= 3; a++) {
                block_coordonate support1 = neighbour + coord3D({a, 0, 0});
                Uint16 id1 = world->get_block_id(support1);
                if (id1 != BLOCK_EMPTY && id1 != BLOCK_WATER) {
                    support_count++;
                    break;
                }
                block_coordonate support2 = neighbour + coord3D({0, a, 0});
                Uint16 id2 = world->get_block_id(support2);
                if (id2 != BLOCK_EMPTY && id2 != BLOCK_WATER) {
                    support_count++;
                    break;
                }
                block_coordonate support3 = neighbour + coord3D({-a, 0, 0});
                Uint16 id3 = world->get_block_id(support3);
                if (id3 != BLOCK_EMPTY && id3 != BLOCK_WATER) {
                    support_count++;
                    break;
                }
                block_coordonate support4 = neighbour + coord3D({0, -a, 0});
                Uint16 id4 = world->get_block_id(support4);
                if (id4 != BLOCK_EMPTY && id4 != BLOCK_WATER) {
                    support_count++;
                    break;
                }
            }
            if (support_count >= 2) has_support = true;
            
            if ( 
                // check if neighbour is in world
                neighbour.chunk.x >= 0 && neighbour.chunk.x <= world->max_chunk_coord.x &&
                neighbour.chunk.y >= 0 && neighbour.chunk.y <= world->max_chunk_coord.y &&
                
                !(
                    // check if block bellow is air
                    world->get_block_id(coord + coord3D({0, 0, -1})) == BLOCK_EMPTY &&
                    
                    // check if there are support blocks (solid blocks on the x and y axis at at most 3 blocks of distance)
                    (!has_support)
                ) &&

                // check if neighbour is empty
                world->chunks[neighbour.chunk.x][neighbour.chunk.y][neighbour.chunk.z].blocks[neighbour.x][neighbour.y][neighbour.z].id == BLOCK_EMPTY
                ) {
                // world_coordonate wcoord = world->convert_coord(neighbour);
                // std::cout << "water spread to " << wcoord.x << ", " << wcoord.y << ", " << wcoord.z << std::endl;
                event_handler->add_event(GAME_EVENT_SINGLE_BLOCK_MOD_ALT, neighbour, BLOCK_WATER);
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
        // world_coordonate wcoord = world->convert_coord(neighbour);
        event_handler->add_event(GAME_EVENT_SINGLE_BLOCK_MOD_ALT, neighbour, BLOCK_WATER);
        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, neighbour);
        engine->add_event(event);
        }
    
    // engine->world_mutex.unlock();
}




bool PhysicsEventWater::operator==(const PhysicsEvent* event) const {
    if (event->get_id() != PHYSICS_EVENT_WATER) return false;
    const PhysicsEventWater* event_cast = static_cast<const PhysicsEventWater*>(event);
    return coord == event_cast->get_coord();
}

bool PhysicsEventWaterCheckChunk::operator==(const PhysicsEvent* other) const { // checks if the event has the same type and chunk coord
    if (other->get_id() != PHYSICS_EVENT_WATER) return false;
    const PhysicsEventWaterCheckChunk* other_cast = static_cast<const PhysicsEventWaterCheckChunk*>(other);
    return chunk == other_cast->get_chunk();
}

bool HasNeighbourWithID(const World* world, const block_coordonate& coord, const int id) {
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue; // skip self
            if (x != 0 && y != 0) continue; // skip diagonals

            block_coordonate neighbour = coord + coord3D({x, y, 0});

            if (
                // check if neighbour is in world
                neighbour.chunk.x >= 0 && neighbour.chunk.x <= world->max_chunk_coord.x &&
                neighbour.chunk.y >= 0 && neighbour.chunk.y <= world->max_chunk_coord.y &&

                // check if neighbour is empty
                world->chunks[neighbour.chunk.x][neighbour.chunk.y][neighbour.chunk.z].blocks[neighbour.x][neighbour.y][neighbour.z].id == id
                ) {
                return true;
            }

        }
    }
    // check {0, 0, -1}
    block_coordonate neighbour = coord + coord3D({0, 0, -1});
    if (
        neighbour.chunk.z >= 0 && neighbour.chunk.z < world->max_chunk_coord.z &&
        world->chunks[neighbour.chunk.x][neighbour.chunk.y][neighbour.chunk.z].blocks[neighbour.x][neighbour.y][neighbour.z].id == id
        ) {
        return true;
    }
    return false;
}

void PhysicsEventWaterCheckChunk::execute() {
    // engine->world_mutex.lock();
    if (world->chunks[chunk.x][chunk.y][chunk.z].compress_value == CHUNK_NON_UNIFORM) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    if (world->chunks[chunk.x][chunk.y][chunk.z].blocks[x][y][z].id == BLOCK_WATER) {
                        
                        if (HasNeighbourWithID(world, block_coordonate({x, y, z, chunk}), BLOCK_EMPTY)) {
                            PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({x, y, z, chunk}));
                            engine->add_event(event);
                        }
                    }
                }
            }
        }

        // check for water in the first blocks of neighbouring chunks
        for (int a = 0; a < CHUNK_SIZE; a++) { // iterate over the face of the chunk that is directly adjacent to the chunk we are checking
            for (int b = 0; b < CHUNK_SIZE; b++) {
                // check chunk above
                if (chunk.z < world->max_chunk_coord.z) {
                    if (world->chunks[chunk.x][chunk.y][chunk.z + 1].blocks[a][b][0].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, b, 0, {chunk.x, chunk.y, chunk.z + 1}}));
                        engine->add_event(event);
                    }
                }
                // check chunk below
                if (chunk.z > 0) {
                    if (world->chunks[chunk.x][chunk.y][chunk.z - 1].blocks[a][b][CHUNK_SIZE - 1].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, b, CHUNK_SIZE - 1, {chunk.x, chunk.y, chunk.z - 1}}));
                        engine->add_event(event);
                    }
                }
                // check chunk to the right
                if (chunk.x < world->max_chunk_coord.x) {
                    if (world->chunks[chunk.x + 1][chunk.y][chunk.z].blocks[0][a][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({0, a, b, {chunk.x + 1, chunk.y, chunk.z}}));
                        engine->add_event(event);
                    }
                }
                // check chunk to the left
                if (chunk.x > 0) {
                    if (world->chunks[chunk.x - 1][chunk.y][chunk.z].blocks[CHUNK_SIZE - 1][a][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({CHUNK_SIZE - 1, a, b, {chunk.x - 1, chunk.y, chunk.z}}));
                        engine->add_event(event);
                    }
                }
                // check chunk in front
                if (chunk.y < world->max_chunk_coord.y) {
                    if (world->chunks[chunk.x][chunk.y + 1][chunk.z].blocks[a][0][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, 0, b, {chunk.x, chunk.y + 1, chunk.z}}));
                        engine->add_event(event);
                    }
                }
                // check chunk behind
                if (chunk.y > 0) {
                    if (world->chunks[chunk.x][chunk.y - 1][chunk.z].blocks[a][CHUNK_SIZE - 1][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, CHUNK_SIZE - 1, b, {chunk.x, chunk.y - 1, chunk.z}}));
                        engine->add_event(event);
                    }
                }
            }
        }
    }
    else if (world->chunks[chunk.x][chunk.y][chunk.z].compress_value == BLOCK_WATER) { // only iterate over the edges and the botom of the chunk
        for (int a = 0; a < CHUNK_SIZE; a++) {
            for (int b = 0; b < CHUNK_SIZE; b++) {
                // check bottom
                if (HasNeighbourWithID(world, block_coordonate({a, b, 0, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, b, 0, chunk}));
                    engine->add_event(event);
                }
                
                // check edges
                if (HasNeighbourWithID(world, block_coordonate({a, 0, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, 0, b, chunk}));
                    engine->add_event(event);
                }
                if (HasNeighbourWithID(world, block_coordonate({a, CHUNK_SIZE - 1, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, CHUNK_SIZE - 1, b, chunk}));
                    engine->add_event(event);
                }
                if (HasNeighbourWithID(world, block_coordonate({0, a, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({0, a, b, chunk}));
                    engine->add_event(event);
                }
                if (HasNeighbourWithID(world, block_coordonate({CHUNK_SIZE - 1, a, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({CHUNK_SIZE - 1, a, b, chunk}));
                    engine->add_event(event);
                }
            }
        }
    }
    // engine->world_mutex.unlock();
}

PhysicsEngine::~PhysicsEngine() {
    while (!event_queue.empty()) {
        delete event_queue.front();
        event_queue.pop_front();
    }
    running = false;
    alive = false;
}

void PhysicsEngine::tick() {
    Uint64 start = Get_time_ms();
   
    // std::cout << "attempting to lock queue_mutex\n";
    queue_mutex.lock();
    // std::cout << "queue_mutex lock in tick" << std::endl;

    std::deque <PhysicsEvent*> event_queue_copy;

    for (PhysicsEvent* event : event_queue) {
        event_queue_copy.push_back(event);
    }

    event_queue.clear();


    // std::cout << "queue_mutex unlock in tick" << std::endl;
    queue_mutex.unlock();

    // std::cout << "attempting to lock world_mutex\n";
    world_mutex.lock();
    // std::cout << "world_mutex lock in tick" << std::endl;  
    // std::cout << "executing " << event_queue_copy.size() << " events" << std::endl;
    while(!event_queue_copy.empty()) {
        event_queue_copy.front()->execute();
        delete event_queue_copy.front();
        event_queue_copy.pop_front();
    }
    // std::cout << "world_mutex unlock in tick" << std::endl;
    world_mutex.unlock();

    Uint64 end = Get_time_ms();
    Uint64 delta = end - start;

    if (delta < tick_delay) {
        // std::cout << "sleeping for " << tick_delay - delta << " ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_delay - delta));
    }
    
}

void PhysicsEngine::add_event(PhysicsEvent* event) {
    // std::cout << "attempting to lock queue_mutex\n";
    queue_mutex.lock();
    // std::cout << "queue_mutex lock in add_event" << std::endl;

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
    // std::cout << "queue_mutex unlock in add_event" << std::endl;
    queue_mutex.unlock();
}

void PhysicsEngine::add_event(int id, void* data) {
    queue_mutex.lock();
    switch (id) {
        case PHYSICS_EVENT_WATER_CHECK_CHUNK: {
            PhysicsEventWaterCheckChunk* event = new PhysicsEventWaterCheckChunk(world, this, event_handler, *(chunk_coordonate*)data);
            event_queue.push_back(event);
            break;
        }
        case PHYSICS_EVENT_WATER_CHECK_BLOCK: {
            PhysicsEventWater* event = new PhysicsEventWater(world, this, event_handler, *(block_coordonate*)data);
            event_queue.push_back(event);
            break;
        }
    }
    queue_mutex.unlock();
}

void PhysicsEngine::clear_events() {
    queue_mutex.lock();
    while (!event_queue.empty()) {
        delete event_queue.front();
        event_queue.pop_front();
    }
    queue_mutex.unlock();
}

int EngineThread(void* arg) {
    PhysicsEngine* engine = (PhysicsEngine*)arg;
    while (engine->alive) {
        if (engine->is_running())
            engine->tick();
        else 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}

PhysicsEngine::PhysicsEngine(World* world, Multithreaded_Event_Handler* event_handler) : world(world), event_handler(event_handler) {
    running = true;
    alive = true;
    thread = SDL_CreateThread(EngineThread, "PhysicsEngine", this);

}