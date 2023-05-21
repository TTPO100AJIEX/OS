#include "rooms.h"

#include <signal.h>
#include <unistd.h>

struct Rooms initialize_rooms(__attribute__ ((unused)) const char* memory_name, const char* semaphore_name, int rooms1, int rooms2)
{
    // Create the object
    struct Rooms answer = {
        .owner = getpid(),
        .ok = false,
        
        .mem = create_memory(memory_name, (rooms1 + rooms2) * sizeof(struct Room)),
        .sem = create_semaphore(semaphore_name, 1),
        .storage = NULL,

        .rooms1 = rooms1,
        .rooms2 = rooms2
    };
    // Check if everything initialized successfully
    if (!answer.mem.mem || answer.sem.id == -1) return answer;
    // Set remaining fields
    answer.storage = answer.mem.mem;
    answer.ok = true;
    // Fill the memory
    for (int i = 0; i < rooms2; i++) { answer.storage[i].residents.people[0] = 0; answer.storage[i].residents.people[1] = 0; }
    for (int i = rooms2; i < rooms1 + rooms2; i++) { answer.storage[i].residents.person = 0; }
    return answer;
}
int delete_rooms(struct Rooms* this)
{
    int status = 0;
    // Kill all remaining visitors in the main process
    if (this->owner == getpid())
    {
        for (int i = 0; i < this->rooms2; i++)
        {
            if (this->storage[i].residents.people[0] && kill(this->storage[i].residents.people[0], SIGINT) == -1) status = -1;
            if (this->storage[i].residents.people[1] && kill(this->storage[i].residents.people[1], SIGINT) == -1) status = -1;
        }
        for (int i = this->rooms2; i < this->rooms1 + this->rooms2; i++)
        {
            if (this->storage[i].residents.person && kill(this->storage[i].residents.person, SIGINT) == -1) status = -1;
        }
    }
    // Delete the memory and the semaphore
    if (delete_memory(&(this->mem)) == -1) status = -1;
    if (delete_semaphore(&(this->sem)) == -1) status = -1;
    return status;
}

int lock_rooms(struct Rooms* this) { return wait_semaphore(&(this->sem)); }
int unlock_rooms(struct Rooms* this) { return post_semaphore(&(this->sem)); }


// Just some nice drawing
static char* draw_id(char* write_iter, pid_t id)
{
    char offset = ' ';
    for (int denom = 10000; denom > 0; denom /= 10)
    {
        if (denom == 1 || id / denom != 0) offset = '0';
        *(write_iter++) = offset + id / denom;
        id %= denom;
    }
    return write_iter;
}
static char* draw_gender(char* write_iter, enum Gender gender)
{
    *(write_iter++) = '(';
    switch (gender)
    {
        case MALE: { *(write_iter++) = 'm'; break; }
        case FEMALE: { *(write_iter++) = 'f'; break; }
        default: { *(write_iter++) = ' '; }
    }
    *(write_iter++) = ')';
    return write_iter;
}
#include <stdlib.h>
char* get_rooms_layout(struct Rooms* this)
{
    char* result = malloc(((this->rooms1 + this->rooms2) * 6 + 2) * 3 + (this->rooms2 * 6 + 2) * 2 + 1);
    char* write_iter = result;

    *(write_iter++) = '-';
    for (int i = 0; i < this->rooms1 + this->rooms2; i++) for (int j = 0; j < 6; j++) *(write_iter++) = '-';
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (int i = 0; i < this->rooms2; i++) { write_iter = draw_id(write_iter, this->storage[i].residents.people[0]); *(write_iter++) = '|'; }
    for (int i = this->rooms2; i < this->rooms1 + this->rooms2; i++) { write_iter = draw_id(write_iter, this->storage[i].residents.person); *(write_iter++) = '|'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (int i = 0; i < this->rooms2; i++) { *(write_iter++) = ' '; *(write_iter++) = ' '; write_iter = draw_gender(write_iter, this->storage[i].gender); *(write_iter++) = '|'; }
    for (int i = this->rooms2; i < this->rooms1 + this->rooms2; i++) { *(write_iter++) = '-'; write_iter = draw_gender(write_iter, this->storage[i].gender); *(write_iter++) = '-'; *(write_iter++) = '-'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (int i = 0; i < this->rooms2; i++) { write_iter = draw_id(write_iter, this->storage[i].residents.people[1]); *(write_iter++) = '|'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '-';
    for (int i = 0; i < this->rooms2; i++) for (int j = 0; j < 6; j++) *(write_iter++) = '-';

    *(write_iter++) = '\n'; *(write_iter++) = '\0';
    return result;
}



static int find_room(struct Rooms* this, enum Gender gender)
{
    // Search rooms for two people
    for (int i = 0; i < this->rooms2; i++)
    {
        if (!this->storage[i].residents.people[0] && !this->storage[i].residents.people[1]) return i; // Empty room for two people
        if (this->storage[i].residents.people[0] && !this->storage[i].residents.people[1] && this->storage[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
        if (!this->storage[i].residents.people[0] && this->storage[i].residents.people[1] && this->storage[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
    }
    // Search rooms for one person
    for (int i = this->rooms2; i < this->rooms1 + this->rooms2; i++)
    {
        if (!this->storage[i].residents.person) return i; // Empty room for one person
    }
    // Nothing has been found
    return -1;
}
int take_room(struct Rooms* this, enum Gender gender)
{
    int room = find_room(this, gender); // Find a room to put the visitor to
    if (room != -1) this->storage[room].gender = gender; // Set the gender of the room
    if (room >= 0 && room < this->rooms2)
    {
        // Take one place
        if (!this->storage[room].residents.people[0]) this->storage[room].residents.people[0] = getpid();
        else this->storage[room].residents.people[1] = getpid();
    }
    if (room >= this->rooms2) this->storage[room].residents.person = getpid(); // Take the place
    return room;
}


int free_room(struct Rooms* this)
{
    // Find where the visitor has been living and remove the data about him
    int answer = -1;
    // Check rooms for two people
    for (int i = 0; i < this->rooms2; i++)
    {
        if (this->storage[i].residents.people[0] == getpid())
        {
            this->storage[i].residents.people[0] = 0;
            if (!this->storage[i].residents.people[1]) this->storage[i].gender = NONE;
            answer = i;
        }
        if (this->storage[i].residents.people[1] == getpid())
        {
            this->storage[i].residents.people[1] = 0;
            if (!this->storage[i].residents.people[0]) this->storage[i].gender = NONE;
            answer = i;
        }
    }
    // Check rooms for one person
    for (int i = this->rooms2; i < this->rooms1 + this->rooms2; i++)
    {
        if (this->storage[i].residents.person == getpid()) { this->storage[i].residents.person = 0; this->storage[i].gender = NONE; answer = i; }
    }
    return answer;
}