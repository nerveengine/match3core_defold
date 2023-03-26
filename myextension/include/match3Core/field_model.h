#pragma once

#include <newmath/index2.h>
#include <vector>
#include <deque>
#include <optional>
#include <functional>

enum class Direction
{
	none	= 0x00u,
	up		= 0x01u,
	right	= 0x02u,
	down	= 0x04u,
	left	= 0x08u,
};

enum class LayerType
{
	Base,
    Spawn,
	Items,
	Direction,
	Portals,
};


struct BaseCell
{
    bool enabled{ true };
};

struct SpawnCell
{
    bool is_spawner{ false };
    /* Направление в котором происходит появление фишки.
     * Например left. Значит фишка выдвинется справа-налево.
     * none - фишка повявляется прямо в ячейке.
     * Отдельная настройка направления для того чтобы этот слой не зависил от остальных.
     * Конечно можно и объеденить
     */
    Direction direction{ Direction::none };
};

struct ItemCell
{
    int item_index{ 0 }; // 0 = no item ?
};



struct DirectionCell
{
    Direction direction{ Direction::down };
};

// Bomb Layers
struct BombBaseCell
{
    bool is_pivot{ false };
};

struct BombMaskCell
{
    bool destroy{ false };
};


struct PortalCell
{
    int portal_id{ -1 }; // -1 = no portal
    bool portal_in{ true };
};

struct Match3Element
{
    int match_id{ 0 };
    size_t uid{ 0 };
};


template<class LayerItem>
struct LayerBase : public std::vector<std::vector<LayerItem>>
{
    void setSize(const nm::index2& size, const LayerItem& item = LayerItem{})
    {
        this->resize(size.x);
        for (auto& col : *this)
        {
            col.resize(size.y, item);
        }
    }

    nm::index2 getSize() const
    {
        return { (int) this->size(), (int) (*this)[0].size() };
    }
};

template<class LayerItem>
void transpose(const std::vector<std::vector<LayerItem>>& data, std::vector<std::vector<LayerItem>>& result)
{
    result = std::vector<std::vector<LayerItem>>(data[0].size(), std::vector<int>(data.size()));
    for (std::size_t i = 0; i < data[0].size(); i++)
    {
        for (std::size_t j = 0; j < data.size(); j++)
        {
            result[i][j] = data[j][i];
        }
    }
}

struct DirectionLayer : public LayerBase<DirectionCell>
{	
};

struct ItemsField : public LayerBase<ItemCell>
{
};

struct BaseFieldLayer : public LayerBase<BaseCell>
{
};

struct SpawnLayer : public LayerBase<SpawnCell>
{
};

struct PortalsLayer : public LayerBase<PortalCell>
{
};

struct IndicesLayer : public LayerBase<int>
{
};


struct Match3ElementsLayer : public LayerBase<Match3Element*>
{
};

struct BombBaseLayer : public LayerBase<BombBaseCell>
{
};

struct BombMaskLayer : public LayerBase<BombMaskCell>
{
};




struct BombField
{
	void setSize(const nm::index2& size)
	{
		base_layer.setSize(size);
		mask_layer.setSize(size);
		size_ = size;
	}
	nm::index2 size() const { // :(
		return size_;
	}
	BombBaseLayer base_layer;
	BombMaskLayer mask_layer;
private:
	nm::index2 size_;
};

struct CompleteMatch
{
    std::vector<Match3Element**> matchedFieldElements;

    void add(Match3Element** c0, Match3Element** c1, Match3Element** c2);
};

struct BaseField
{
    void resize(int w, int h);

    nm::index2 size() const;

    BaseFieldLayer base;
    SpawnLayer spawns;
    DirectionLayer direction;
    PortalsLayer portals;
    ItemsField items;
};

struct ComplexField : BaseField
{
    
};

struct GameArea : BaseField
{
    GameArea() = default;
    GameArea(const GameArea& area);

    void clear();
    void resize(int w, int h);
    void setChipTypesRange(int from, int to);

    IndicesLayer game_field_mapping() const;
    void remap(const IndicesLayer& mapping);

    Match3ElementsLayer game_field;
    std::deque<Match3Element> list;

    int from{ 1 };
    int to{ 5 };

    Match3Element** selection{ nullptr };
};


enum class FillStrategy { FillEmptyRandom, FillEmptyRandomWithoutMatches };

//
GameArea create_gameArea(const ComplexField& field);
void fillEmpty(GameArea& gameArea, FillStrategy fillStrategy);
CompleteMatch find_matches(GameArea& gameArea);

//
void clear_matched_elemets(const CompleteMatch& matches);
std::size_t try_clear_matches(GameArea& gameArea);

//mechanics
bool apply_gravity(GameArea& gameArea);
bool spawn_chips(GameArea& gameArea);
bool apply_mechanics(GameArea& gameArea);

//evaluator
size_t evaluate_field(GameArea& gameArea, bool clearMatches);


