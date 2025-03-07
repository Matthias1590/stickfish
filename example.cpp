#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>
#include <assert.h>
#include <chrono>

#define TIME_PER_MOVE_MS 10000

typedef enum {
	WAITING,
	SEARCHING,
} t_state;

t_state state;
std::chrono::_V2::system_clock::time_point searchStartTime;

bool inputAvailable() {
	struct pollfd fds;
	fds.fd = 0;
	fds.events = POLLIN;
	fds.revents = 0;

	return poll(&fds, 1, 0) > 0;
}

std::string readCommand() {
	std::string input;
	std::getline(std::cin, input);
	return input;
}

void updateState();

int searchDepth(int depth) {
	updateState();
	if (state != SEARCHING) {
		std::cout << "cancelling inside searchDepth" << std::endl;
		return 0;
	}

	sleep(1);
	if (depth == 0) {
		return rand() % 100;
	}
	return searchDepth(depth - 1);
}

void startSearch() {
	state = SEARCHING;
	searchStartTime = std::chrono::system_clock::now();

	int savedScore = 0;
	int depth = 4;
	while (state == SEARCHING) {
		std::cout << "info depth " << depth << std::endl;

		int score = searchDepth(depth);
		if (state == SEARCHING) {
			savedScore = score;
		}

		depth++;

		if (depth > 10) {
			break;
		}
	}

	std::cout << "bestmove a1a2 with score " << savedScore << std::endl;
	state = WAITING;
}

void updateState() {
	switch (state) {
	case WAITING: {
		if (!inputAvailable()) {
			return;
		}
		auto command = readCommand();
		
		if (command == "go") {
			startSearch();
		}
	} break;
	case SEARCHING: {
		auto searchedTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - searchStartTime).count();

		if (searchedTimeMs >= TIME_PER_MOVE_MS) {
			std::cout << "cancelling search because of time" << std::endl;
			state = WAITING;
		}

		if (!inputAvailable()) {
			return;
		}
		auto command = readCommand();

		if (command == "stop") {
			std::cout << "cancelling search because of stop" << std::endl;
			state = WAITING;
		}
	} break;
	}
}

int main() {
	while (true) {
		updateState();
	}

	return 0;
}
