#include "rooms.h"



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