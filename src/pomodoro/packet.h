#include "Arduino.h"

typedef struct packet_t {
	// 0: no op, 1: start, 2: stop, 3: pause, 4: resume, 5: reset, 6: data, 7: notify
	char *command;
	uint16_t remaining_time;
};

void parse_packet(uint8_t *data, packet_t *packet);