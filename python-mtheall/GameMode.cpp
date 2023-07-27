#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *GameMode::Type = nullptr;

PyType_Slot GameMode::Slots[] = {
    {0, nullptr},
};

PyType_Spec GameMode::Spec = {
    .name      = "RocketSim.GameMode",
    .basicsize = sizeof (GameMode),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = GameMode::Slots,
};
}
