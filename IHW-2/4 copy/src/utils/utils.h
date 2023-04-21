#pragma once

void sleep_milliseconds(unsigned int milliseconds);

#include "../protocol.h"
int request(struct State state, struct Message message, struct Message* response);
int response(struct State state, struct Message message);


struct State init_state();
void clear_state(struct State state);
void close_state(struct State state);