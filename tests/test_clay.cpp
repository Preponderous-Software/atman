// Clay and Pots: throwing at the wheel takes a few seconds standing there and
// ends with a name; the clay lens comes after three pots; breaking a pot
// removes its name and moves no mote of the space it held.
#include <cmath>
#include <cstring>

#include "check.h"
#include "sim/clay.h"
#include "sim/valley.h"
#include "sim/world.h"

using namespace atman;
namespace V = atman::valley;

namespace {

// Beside the wheel, on the board's side, facing the board.
World atWheelSide() {
    World w = newWorld(1);
    w.player.x = 9.0f;
    w.player.z = -12.2f;
    return w;
}

void idle(World& w, int ticks, Input in = {}) {
    in.yaw = w.player.yaw;
    in.pitch = w.player.pitch;
    for (int t = 0; t < ticks; ++t) tick(w, in);
}

// Throws `form` and names it, standing still the whole time.
void throwPot(World& w, Form form, const char* name) {
    Input start;
    start.throwForm = static_cast<int32_t>(form);
    idle(w, 1, start);
    idle(w, static_cast<int>(kThrowSeconds * kTickRate) + 1);
    Input named;
    named.name = name;
    idle(w, 1, named);
}

// Turns to look at the pot in `slot`, from where the player stands.
void lookAtPot(World& w, int slot) {
    const FormShape& f = formShape(w.pots[slot].form);
    const Vec3 b = potBase(slot);
    const float dx = b.x - w.player.x, dz = b.z - w.player.z;
    const float dy = b.y + f.height / 2 - (V::walkHeight(w.player.x, w.player.z) + V::kEyeHeight);
    w.player.yaw = std::atan2(dx, -dz);
    w.player.pitch = std::atan2(dy, std::sqrt(dx * dx + dz * dz));
}

bool same(const std::array<Vec3, V::kMoteCount>& a, const std::array<Vec3, V::kMoteCount>& b) {
    for (int i = 0; i < V::kMoteCount; ++i)
        if (a[i].x != b[i].x || a[i].y != b[i].y || a[i].z != b[i].z) return false;
    return true;
}

void breakLookedAt(World& w) {
    Input in;
    in.breakPot = true;
    idle(w, 1, in);
}

}  // namespace

int main() {
    // The board and its slots: solid, beside the wheel, within reach of the
    // spot by the wheel, and each slot on the board.
    {
        CHECK(!V::blocked(9.0f, -12.2f));
        CHECK(V::blocked(12.0f, -11.0f));
        for (int i = 0; i < V::kBoardSlots; ++i) CHECK(V::kBoard.contains(V::boardSlot(i).x, V::boardSlot(i).z));
        CHECK(V::boardTop() > V::groundHeight(12.0f, -11.0f));
    }

    // Throwing takes kThrowSeconds at the wheel; then the pot waits for a name.
    {
        World w = atWheelSide();
        CHECK(atWheel(w));
        Input start;
        start.throwForm = static_cast<int32_t>(Form::Bowl);
        idle(w, 1, start);
        CHECK(w.throwing.active);
        idle(w, static_cast<int>(kThrowSeconds * kTickRate) - 5);
        CHECK(w.potsMade == 0);
        idle(w, 6);
        CHECK(w.potsMade == 1);
        CHECK(w.pots[0].made && w.pots[0].form == Form::Bowl);
        CHECK(w.naming == 0);
        // Nothing else can be thrown until it has a name.
        idle(w, 1, start);
        CHECK(!w.throwing.active);
        Input named;
        named.name = "  Rice for Amma  ";
        idle(w, 1, named);
        CHECK(w.naming == -1);
        CHECK(std::strcmp(w.pots[0].name.data(), "Rice for Amma") == 0);
        CHECK(inventory(w).size() == 1 && inventory(w)[0] == "Rice for Amma");
    }

    // Names: an empty one falls back to the form; odd bytes are dropped; long
    // names are cut to kNameLength.
    {
        World w = atWheelSide();
        throwPot(w, Form::Cup, "   ");
        CHECK(std::strcmp(w.pots[0].name.data(), "Cup") == 0);
        throwPot(w, Form::Lamp, "Di\x01ya\xc3\xa9");
        CHECK(std::strcmp(w.pots[1].name.data(), "Diya") == 0);
        throwPot(w, Form::WaterJar, "A very long name for a water jar indeed");
        CHECK(std::strlen(w.pots[2].name.data()) == static_cast<size_t>(kNameLength));
    }

    // Away from the wheel nothing starts, and walking off mid-throw loses it.
    {
        World w = newWorld(1);
        CHECK(!atWheel(w));
        Input start;
        start.throwForm = 0;
        idle(w, 1, start);
        CHECK(!w.throwing.active);

        World m = atWheelSide();
        idle(m, 1, start);
        idle(m, kTickRate);
        CHECK(m.throwing.active);
        for (int t = 0; t < 3 * kTickRate; ++t) {
            Input in;
            in.yaw = kPi / 2;  // east, away from the wheel
            in.forward = 1;
            tick(m, in);
        }
        CHECK(!atWheel(m));
        CHECK(!m.throwing.active);
        CHECK(m.potsMade == 0);
    }

    // The board holds kBoardSlots pots, and then it is full.
    {
        World w = atWheelSide();
        for (int i = 0; i < V::kBoardSlots + 2; ++i) throwPot(w, Form::Cup, "");
        CHECK(w.potsMade == V::kBoardSlots);
        CHECK(inventory(w).size() == static_cast<size_t>(V::kBoardSlots));
    }

    // The clay lens comes after three pots, and toggles.
    {
        World w = atWheelSide();
        Input lens;
        lens.toggleLens = true;
        throwPot(w, Form::WaterJar, "Ganga water");
        throwPot(w, Form::Bowl, "");
        idle(w, 1, lens);
        CHECK(!lensAvailable(w));
        CHECK(!w.clayLens);
        throwPot(w, Form::Lamp, "");
        CHECK(lensAvailable(w));
        CHECK(!w.codex[static_cast<int>(CodexId::ClayAndPots)]);
        idle(w, 1, lens);
        CHECK(w.clayLens);
        CHECK(w.codex[static_cast<int>(CodexId::ClayAndPots)]);
        idle(w, 1, lens);
        CHECK(!w.clayLens);
    }

    // Pot-space. The motes are inside the box over the board, and some hang
    // where a water jar stands, inside it.
    {
        const auto& m = motes();
        const Box3& b = V::kMoteSpace;
        int insideJar = 0;
        const FormShape& jar = formShape(Form::WaterJar);
        const Vec3 base = potBase(0);
        for (const Vec3& p : m) {
            CHECK(p.x >= b.lo.x && p.x <= b.hi.x && p.z >= b.lo.z && p.z <= b.hi.z);
            const float g = V::groundHeight(p.x, p.z);
            CHECK(p.y >= g + b.lo.y && p.y <= g + b.hi.y + 1e-4f);
            const float dx = p.x - base.x, dz = p.z - base.z;
            if (dx * dx + dz * dz < jar.radius * jar.radius && p.y > base.y && p.y < base.y + jar.height) ++insideJar;
        }
        CHECK(insideJar > 0);
    }

    // Breaking a pot: it is still on the board as shards, its name leaves
    // the inventory, and not one mote moves. Nothing is rewarded.
    {
        World w = atWheelSide();
        const auto before = motes();
        throwPot(w, Form::WaterJar, "Ganga water");
        throwPot(w, Form::Bowl, "Rice");
        CHECK(same(motes(), before));
        w.player.x = 10.1f;
        w.player.z = -12.2f;
        lookAtPot(w, 0);
        CHECK(lookedAtPot(w) == 0);
        const int32_t standing = w.standing;
        const auto quests = w.questDone;
        breakLookedAt(w);
        CHECK(w.pots[0].broken);
        CHECK(w.pots[0].made);
        CHECK(!w.pots[1].broken);
        CHECK(inventory(w).size() == 1 && inventory(w)[0] == "Rice");
        CHECK(same(motes(), before));
        CHECK(w.codex[static_cast<int>(CodexId::PotSpace)]);
        CHECK(w.standing == standing);
        CHECK(w.questDone == quests);
        // A broken pot is not a target any more; nor is anything out of reach.
        CHECK(lookedAtPot(w) == -1);
        w.player.x = 10.1f;
        w.player.z = -14.5f;
        lookAtPot(w, 1);
        CHECK(lookedAtPot(w) == -1);
        breakLookedAt(w);
        CHECK(!w.pots[1].broken);
    }

    // Looking at nothing breaks nothing.
    {
        World w = atWheelSide();
        throwPot(w, Form::Cup, "");
        w.player.yaw = -kPi / 2;
        breakLookedAt(w);
        CHECK(!w.pots[0].broken);
    }

    TEST_MAIN_END();
}
