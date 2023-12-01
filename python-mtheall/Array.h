#pragma once

#include "PyRef.h"

#include <numpy/arrayobject.h>

#include <cstddef>

namespace RocketSim::Python
{
class PyArrayRef final : public PyRef<PyArrayObject>
{
public:
	PyArrayRef () noexcept = delete;

	PyArrayRef (std::nullptr_t) noexcept;

	PyArrayRef (unsigned dim0_, unsigned dim1_ = 0) noexcept;

	float &operator() (unsigned dim0_, unsigned dim1_ = 0) noexcept;

	float const &operator() (unsigned dim0_, unsigned dim1_ = 0) const noexcept;

	bool isnan () const noexcept;

private:
	unsigned const m_dim0;
	unsigned const m_dim1;
};
}
