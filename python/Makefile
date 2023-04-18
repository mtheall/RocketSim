.PHONY: all build wheel install clean

all: build

build:
	python3 ../setup.py build --debug

wheel: build
	python3 ../setup.py bdist_wheel --py-limited-api=cp34

install: wheel
	pip3 install --force-reinstall --user ./dist/RocketSim-1.2.0a6-cp34-abi3-linux_x86_64.whl

clean:
	$(RM) -r build/ dist/ RocketSim.egg-info/
