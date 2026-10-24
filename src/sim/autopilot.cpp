#include "sim/autopilot.h"

#include <algorithm>
#include <limits>

#include "sim/chores.h"
#include "sim/clay.h"
#include "sim/valley.h"

namespace atman {

namespace V = atman::valley;

namespace {

constexpr float kArrive = 0.25f;      // close enough to a route point
constexpr float kAround = 0.8f;       // route points stand this far off an obstacle
constexpr float kSample = 0.2f;       // a straight way is checked every this many metres
constexpr int kStallTicks = 2 * kTickRate;
constexpr float kCoilApproach = 2.5f;  // look at a snake from this far, east of it, on the path

// Names for the playthrough's pots, by board slot.
const char* const kNames[] = {"Ganga water", "Rice for Amma", "Evening lamp", "Cup", "Cup", "Cup"};

float dist(Vec2 a, Vec2 b) {
    const float dx = a.x - b.x, dz = a.z - b.z;
    return std::sqrt(dx * dx + dz * dz);
}

Vec2 here(const World& w) { return {w.player.x, w.player.z}; }

bool clear(Vec2 a, Vec2 b) {
    const float d = dist(a, b);
    const int n = static_cast<int>(d / kSample) + 1;
    for (int i = 1; i <= n; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(n);
        if (V::blocked(a.x + (b.x - a.x) * t, a.z + (b.z - a.z) * t)) return false;
    }
    return true;
}

// Points around every obstacle, and which pairs see each other: built once.
struct Graph {
    std::vector<Vec2> nodes;
    std::vector<std::vector<char>> sees;
};

const Graph& graph() {
    static const Graph g = [] {
        Graph out;
        auto box = [&](const Box2& b) {
            for (float x : {b.x0 - kAround, b.x1 + kAround})
                for (float z : {b.z0 - kAround, b.z1 + kAround}) out.nodes.push_back({x, z});
        };
        auto circle = [&](Vec2 c, float r) {
            for (int k = 0; k < 8; ++k) {
                const float a = static_cast<float>(k) * kPi / 4;
                out.nodes.push_back({c.x + (r + kAround) * detCos(a), c.z + (r + kAround) * detSin(a)});
            }
        };
        for (const V::House& h : V::kHouses) box(h.footprint);
        box(V::kKiln);
        box(V::kBoard);
        circle(V::kWell, V::kWellRadius);
        circle(V::kWheel, V::kWheelRadius);
        circle(V::kBirdTree, V::kTrunkRadius);
        for (Vec2 t : V::kTrees) circle(t, V::kTreeTrunkRadius);
        out.nodes.erase(std::remove_if(out.nodes.begin(), out.nodes.end(),
                                       [](Vec2 p) { return V::blocked(p.x, p.z); }),
                        out.nodes.end());
        const size_t n = out.nodes.size();
        out.sees.assign(n, std::vector<char>(n, 0));
        for (size_t i = 0; i < n; ++i)
            for (size_t j = i + 1; j < n; ++j)
                out.sees[i][j] = out.sees[j][i] = clear(out.nodes[i], out.nodes[j]) ? 1 : 0;
        return out;
    }();
    return g;
}

Vec2 riverSpot() { return {V::kDrawWaterX + 0.6f, 0.0f}; }
Vec2 potterDoorstep() {
    const Vec2 d = V::kHouses[V::PotterHouse].door;
    return {d.x, d.z + 1.0f};  // the door is on the south wall
}
Vec2 wellSpot() { return {V::kWell.x, V::kWell.z + 1.8f}; }
Vec2 wheelSpot() { return {9.0f, -12.2f}; }
Vec2 potSpot() { return {10.1f, -12.3f}; }
Vec2 ghatTop() { return {23.5f, 0.0f}; }
Vec2 restSpot() { return {24.5f, 1.5f}; }
float pathZ(float x) { return 4.0f - 0.2f * (x - 20.0f); }
Vec2 coilApproach(int i) {
    const float x = V::kCoils[i].x + kCoilApproach;
    return {x, pathZ(x)};
}

// Yaw and pitch from the player's eyes to a point.
void aim(const World& w, Vec3 at, Input& in) {
    const float dx = at.x - w.player.x, dz = at.z - w.player.z;
    const float dy = at.y - (V::walkHeight(w.player.x, w.player.z) + V::kEyeHeight);
    in.yaw = detAtan2(dx, -dz);
    in.pitch = detAtan2(dy, std::sqrt(dx * dx + dz * dz));
}

}  // namespace

std::vector<Vec2> planRoute(Vec2 from, Vec2 to) {
    if (clear(from, to)) return {to};
    const Graph& g = graph();
    const size_t n = g.nodes.size();
    // Dijkstra over the nodes, with `from` and `to` joined to the nodes they see.
    const float inf = std::numeric_limits<float>::max();
    std::vector<float> cost(n, inf);
    std::vector<int> prev(n, -1);
    std::vector<char> doneNode(n, 0), reachesTo(n, 0);
    for (size_t i = 0; i < n; ++i) {
        if (clear(from, g.nodes[i])) cost[i] = dist(from, g.nodes[i]);
        reachesTo[i] = clear(g.nodes[i], to) ? 1 : 0;
    }
    int bestEnd = -1;
    float bestCost = inf;
    for (;;) {
        int u = -1;
        for (size_t i = 0; i < n; ++i)
            if (!doneNode[i] && cost[i] < inf && (u < 0 || cost[i] < cost[static_cast<size_t>(u)])) u = static_cast<int>(i);
        if (u < 0) break;
        const auto uu = static_cast<size_t>(u);
        doneNode[uu] = 1;
        if (reachesTo[uu] && cost[uu] + dist(g.nodes[uu], to) < bestCost) {
            bestCost = cost[uu] + dist(g.nodes[uu], to);
            bestEnd = u;
        }
        for (size_t v = 0; v < n; ++v) {
            if (doneNode[v] || !g.sees[uu][v]) continue;
            const float c = cost[uu] + dist(g.nodes[uu], g.nodes[v]);
            if (c < cost[v]) {
                cost[v] = c;
                prev[v] = u;
            }
        }
    }
    std::vector<Vec2> route;
    if (bestEnd < 0) return {to};  // nothing better: head straight and slide
    for (int i = bestEnd; i >= 0; i = prev[static_cast<size_t>(i)]) route.push_back(g.nodes[static_cast<size_t>(i)]);
    std::reverse(route.begin(), route.end());
    route.push_back(to);
    return route;
}

void Autopilot::walkTo(const World& w, Vec2 to, bool hurry, Input& in) {
    const Vec2 p = here(w);
    if (!hasGoal_ || dist(goal_, to) > 0.01f) {
        goal_ = to;
        hasGoal_ = true;
        route_ = planRoute(p, to);
        best_ = route_.empty() ? 0 : dist(p, route_.front());
        stalled_ = 0;
    }
    while (!route_.empty() && dist(p, route_.front()) < kArrive) {
        route_.erase(route_.begin());
        best_ = route_.empty() ? 0 : dist(p, route_.front());
        stalled_ = 0;
    }
    if (route_.empty()) return;
    const float d = dist(p, route_.front());
    if (d < best_ - 0.02f) {
        best_ = d;
        stalled_ = 0;
    } else if (++stalled_ > kStallTicks) {
        route_ = planRoute(p, to);  // something was in the way: think again from here
        best_ = route_.empty() ? 0 : dist(p, route_.front());
        stalled_ = 0;
    }
    if (route_.empty()) return;
    const Vec2 next = route_.front();
    in.forward = 1;
    in.hurry = hurry;
    in.yaw = detAtan2(next.x - p.x, -(next.z - p.z));
    in.pitch = 0;
}

Input Autopilot::next(const World& w) {
    Input in;
    in.yaw = w.player.yaw;
    in.pitch = w.player.pitch;
    const bool hurry = !w.after;  // the playthrough has an afternoon to fit in; the after-mode does not hurry
    auto arrived = [&](Vec2 at) { return dist(here(w), at) < kArrive; };
    auto goUse = [&](Vec2 at, const char* what) {
        doing_ = what;
        if (arrived(at)) in.use = true;
        else walkTo(w, at, hurry, in);
    };

    if (w.sleep == Sleep::Deep) {
        doing_ = "in deep sleep";
        return in;
    }
    if (w.sleep == Sleep::Dream) {
        doing_ = "dreaming";
        if (!arrived(ghatTop())) walkTo(w, ghatTop(), false, in);
        else {
            in.yaw = kPi / 2;
            in.pitch = 0;
        }
        return in;
    }

    // The lamp: lit after dark, out by day.
    if (w.lampLit != isDusk(w)) in.toggleLamp = true;

    if (w.naming >= 0) {
        doing_ = "naming a pot";
        in.name = kNames[w.naming];
        return in;
    }
    if (w.carry == Carry::Clay) {
        goUse(wheelSpot(), "taking clay to the wheel");
        return in;
    }
    if (w.carry == Carry::Water) {
        goUse(potterDoorstep(), "carrying water to the potter");
        return in;
    }
    if (!isDusk(w)) {
        // The body first, then the day's chores.
        if (w.needs.thirst > 0.6f) {
            goUse(wellSpot(), "going to the well");
            return in;
        }
        if (w.needs.hunger > 0.6f) {
            goUse(yourDoorstep(), "going home to eat");
            return in;
        }
        if (!w.questDone[static_cast<int>(QuestId::FetchClay)]) {
            goUse(V::kClayBank, "going for clay");
            return in;
        }
        if (!w.questDone[static_cast<int>(QuestId::CarryWater)]) {
            goUse(riverSpot(), "going for water");
            return in;
        }
    }
    if (!w.after) {
        // The teachings, in the order an afternoon allows.
        if (w.potsMade < kPotsForLens || w.throwing.active) {
            doing_ = "at the wheel";
            if (!arrived(wheelSpot())) walkTo(w, wheelSpot(), hurry, in);
            else if (!w.throwing.active) in.throwForm = w.potsMade % 3;
            return in;
        }
        if (w.clayLens || !w.codex[static_cast<int>(CodexId::ClayAndPots)]) {
            doing_ = "looking through the clay lens";
            in.toggleLens = true;
            return in;
        }
        if (!w.codex[static_cast<int>(CodexId::PotSpace)]) {
            doing_ = "breaking a pot";
            if (!arrived(potSpot())) {
                walkTo(w, potSpot(), hurry, in);
                return in;
            }
            const Vec3 b = potBase(0);
            aim(w, {b.x, b.y + formShape(w.pots[0].form).height / 2, b.z}, in);
            in.breakPot = lookedAtPot(w) == 0;
            return in;
        }
        if (!isDusk(w)) {
            doing_ = "waiting for the sunset";
            if (!arrived(ghatTop())) walkTo(w, ghatTop(), hurry, in);
            else {
                in.yaw = kPi / 2;
                in.pitch = 0;
            }
            return in;
        }
        if (!w.fearRetired) {
            int nearest = -1;
            for (int i = 0; i < V::kCoilCount; ++i)
                if (!w.coils[i].seenThrough &&
                    (nearest < 0 || dist(here(w), coilApproach(i)) < dist(here(w), coilApproach(nearest))))
                    nearest = i;
            doing_ = "looking closely at a snake";
            if (!arrived(coilApproach(nearest))) {
                walkTo(w, coilApproach(nearest), hurry, in);
                return in;
            }
            const Vec2 c = V::kCoils[nearest];
            aim(w, {c.x, V::groundHeight(c.x, c.z) + 0.1f, c.z}, in);
            return in;
        }
    }
    if (w.needs.thirst > 0.6f) {
        goUse(wellSpot(), "going to the well");
        return in;
    }
    if (isDusk(w)) {
        goUse(yourDoorstep(), "going home to sleep");
        return in;
    }

    doing_ = "sitting on the ghat steps";
    if (!arrived(restSpot())) walkTo(w, restSpot(), hurry, in);
    else {
        in.yaw = kPi / 2;
        in.pitch = -0.1f;
    }
    return in;
}

bool playthroughDone(const World& w) {
    const int minute = static_cast<int>(gameMinutes(w)) % 1440;
    return w.after && w.questDone[0] && w.questDone[1] && minute >= 12 * 60;
}

}  // namespace atman
