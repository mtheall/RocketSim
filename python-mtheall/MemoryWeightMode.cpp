#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *MemoryWeightMode::Type = nullptr;

PyType_Slot MemoryWeightMode::Slots[] = {
    {0, nullptr},
};

PyType_Spec MemoryWeightMode::Spec = {
    .name      = "RocketSim.MemoryWeightMode",
    .basicsize = sizeof (MemoryWeightMode),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = MemoryWeightMode::Slots,
};
}
