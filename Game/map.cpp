#include "map.h"

using namespace game;

/*
class MapSearchNode : public Coords
{
public:
    const Map* map_ = nullptr;

    MapSearchNode(int px, int py, const Map* map) : map_(map), Coords{px, py} {}
    MapSearchNode() : Coords{ 0, 0 }, map_(nullptr) {}

    float GoalDistanceEstimate(MapSearchNode &nodeGoal) {
        return abs(x - nodeGoal.x) + abs(y - nodeGoal.y);
    }

    bool IsGoal(MapSearchNode &nodeGoal) {
        return (x == nodeGoal.x) && (y == nodeGoal.y);
    }

    bool GetSuccessors(AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node) {
        int parent_x = -1;
        int parent_y = -1;

        if (parent_node)
        {
            parent_x = parent_node->x;
            parent_y = parent_node->y;
        }
        MapSearchNode NewNode;
        // push each possible move except allowing the search to go backwards
        if ((map_->getTile(x - 1, y).terrain < MOUNTAIN) && !((parent_x == x - 1) && (parent_y == y)))
        {
            NewNode = MapSearchNode(x - 1, y, map_);
            astarsearch->AddSuccessor(NewNode);
        }

        if ((map_->getTile(x, y - 1).terrain < MOUNTAIN) && !((parent_x == x) && (parent_y == y - 1)))
        {
            NewNode = MapSearchNode(x, y - 1, map_);
            astarsearch->AddSuccessor(NewNode);
        }

        if ((map_->getTile(x + 1, y).terrain < MOUNTAIN) && !((parent_x == x + 1) && (parent_y == y)))
        {
            NewNode = MapSearchNode(x + 1, y, map_);
            astarsearch->AddSuccessor(NewNode);
        }

        if ((map_->getTile(x, y + 1).terrain < MOUNTAIN) && !((parent_x == x) && (parent_y == y + 1)))
        {
            NewNode = MapSearchNode(x, y + 1, map_);
            astarsearch->AddSuccessor(NewNode);
        }

        return true;
    }
    float GetCost(MapSearchNode &successor) {
        return (float)(map_->getTile(x, y).terrain);
    }
    bool IsSameState(MapSearchNode &rhs) {
        // same state in a maze search is simply when (x,y) are the same
        if ((x == rhs.x) &&
            (y == rhs.y))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

*/

Map::Map(size_t width, size_t height) : width_(width), height_(height)
{
    tiles_.assign(width * height, { Terrain::GROUND, false });
}

Tile Map::getTile(const Coords& coords) const
{
    if (!isValidCoord(coords))
        return {MOUNTAIN, true};
    return tiles_.at(coordsToIndex(coords));
}

void game::Map::updateOccupied(const UnitsHolder& units)
{
    for (auto& tile : tiles_)
        tile.isOccupied = false;

    for (auto it = units.begin(); it != units.end(); ++it) {
        const auto& unit = it->second;
        const auto& pos = unit->getCoords();
        if (!isValidCoord(pos))
            continue;
        tiles_[pos.row * width_ + pos.col].isOccupied = true;
    }
}

std::vector<Coords> Map::getPath(const Coords& start, const Coords& goal) const
{
    std::vector<Coords> res;

    /*
    AStarSearch<MapSearchNode> astarsearch;

    unsigned int SearchCount = 0;

    const unsigned int NumSearches = 1;

    while (SearchCount < NumSearches)
    {

        // Create a start state
        MapSearchNode nodeStart(0, 0, this);

        // Define the goal state
        MapSearchNode nodeEnd(10, 10, this);

        // Set Start and goal states

        astarsearch.SetStartAndGoalStates(nodeStart, nodeEnd);

        unsigned int SearchState;
        unsigned int SearchSteps = 0;

        do
        {
            SearchState = astarsearch.SearchStep();
            SearchSteps++;
        } while (SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING);

        if (SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED)
        {
            MapSearchNode *node = astarsearch.GetSolutionStart();

            int steps = 0;

            for (;; )
            {
                node = astarsearch.GetSolutionNext();
                if (!node)
                {
                    break;
                }

                res.push_back(*node);

                steps++;

            };

            // Once you're done with the solution you can free the nodes up
            astarsearch.FreeSolutionNodes();
        }
        else if (SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED)
        {
            cout << "Failed to find path\n";
        }

        // Display the number of loops the search went through
        SearchCount++;
        astarsearch.EnsureMemoryFreed();
    }
    */

    return res;
}

size_t game::Map::width() const
{
    return width_;
}

size_t game::Map::height() const
{
    return height_;
}

size_t Map::coordsToIndex(const Coords& coords) const
{
    return coords.row * width_ + coords.col;
}

bool game::Map::isValidCoord(const Coords& pos) const
{
    return pos.col >= 0 && pos.col < width_ && pos.row >= 0 && pos.row < height_;
}


