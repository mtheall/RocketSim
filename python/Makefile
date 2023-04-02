.PHONY: all wheel install clean

all:
	python3 ../setup.py build

wheel:
	python3 ../setup.py bdist_wheel --py-limited-api=cp34

install: wheel
	pip3 install --force-reinstall --user ./dist/RocketSim-1.0-cp34-abi3-linux_x86_64.whl

clean:
	$(RM) -r build/
