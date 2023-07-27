#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *DemoMode::Type = nullptr;

PyType_Slot DemoMode::Slots[] = {
    {0, nullptr},
};

PyType_Spec DemoMode::Spec = {
    .name      = "RocketSim.DemoMode",
    .basicsize = sizeof (DemoMode),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = DemoMode::Slots,
};
}
