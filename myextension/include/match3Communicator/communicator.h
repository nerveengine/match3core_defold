#pragma once

#include "match3Core/library.include.h"

void run_update_loop();
std::optional<GameArea> get_updated_area();
void push_updated_area(const GameArea& area);
void send_game_area(const GameArea& area);
void setup_on_game_area( std::function<void (const GameArea& area)> );

