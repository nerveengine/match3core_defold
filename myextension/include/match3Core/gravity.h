#pragma once

class GameArea;

namespace m3
{
    /** @brief применяет один шаг гравитации.
      * @returns были ли изменения в поле после применения
      */
    bool apply_gravity_step(GameArea& game_area);
}
