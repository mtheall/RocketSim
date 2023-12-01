.PHONY: all build wheel install clean

PYTHON ?= python3

all: build

build:
	$(PYTHON) ../setup.py build --debug

wheel: build
	$(PYTHON) ../setup.py bdist_wheel --py-limited-api=cp34

install: wheel
	$(PYTHON) -m pip install --force-reinstall --user ./dist/RocketSim-2.0.0.post1-cp34-abi3-linux_x86_64.whl

clean:
	$(RM) -r build/ dist/ RocketSim.egg-info/
