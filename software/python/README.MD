# Testing utilities for map manipulation
Jupyter notebooks used during development for some functionality. Some of the notebooks are published as the help to visualize the inner-workings of the map rendering/loading algorithm.

## Setup
The requirements are provided as a conda environment
```
conda env create -f environment.yml
```
To run the notebooks, activate the environment and launch the jupyter server
```
conda activate bike-companion-32
cd notebooks && jupyter notebook
```

## Notebooks
```
test_bounding_box           Plots colliding tiles given a bounding box
test_bresenham              Plots lines with a certain thickness (modified bresenham)
test_plot_binary_map        Plots a complete BIN map
test_plot_partial_map       Plots a defined part of a BIN map
```

## Reading from the binary file
Python-equivalent implementations to read the map header and a given tile can be found under **src/utils.py**.
```
read_header     Reads the header of the binary map file and returns it as a dictionary
read_tile       Reads a tile with a given ID and returns it as a dictionary (each value representing a road segment)
```
