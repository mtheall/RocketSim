#include "Module.h"

#include <cstddef>
#include <cstring>

namespace RocketSim::Python
{
PyTypeObject *Team::Type = nullptr;

PyType_Slot Team::Slots[] = {
    {0, nullptr},
};

PyType_Spec Team::Spec = {
    .name      = "RocketSim.Team",
    .basicsize = sizeof (Team),
    .itemsize  = 0,
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots     = Team::Slots,
};
}
