#include "packet.h"

void parse_packet(uint8_t *data, packet_t *packet) {
	char *cmd;
	
	if (data[0] == 0x00) {
		cmd = "noop";
	} else if (data[0] == 0x01) {
		cmd = "start";
	} else if (data[0] == 0x02) {
		cmd = "stop";
	} else if (data[0] == 0x03) {
		cmd = "pause";
	} else if (data[0] == 0x04) {
		cmd = "resume";
	}	else if (data[0] == 0x05) {
		cmd = "reset";
	} else if (data[0] == 0x06) {
		cmd = "data";
	} else if (data[0] == 0x07) {
		cmd = "notify";
	} else if (data[0] == 0x08) {
		cmd = "ask";
	} else if (data[0] == 0x09){
		cmd = "ack";
	}

	uint16_t remaining_time = *(data + 1) << 8 | *(data + 2);
	packet->command = cmd;
	packet->remaining_time = remaining_time;
}
