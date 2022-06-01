`plot_performance` reads Digistring's performance output files and generated a plot showing the different performance points. 


# Build instructions
## Requirements
Python 3 and Matplotlib  
Furthermore, `python3` should be present in the environment variable PATH of the environment running Digistring.

On Arch Linux:  
`sudo pacman -S python3 python-matplotlib`


# Usage instructions
To generate performance graph, run:  
`./plot_performance <performance file> [ask_labels]`  
If `ask_labels` is passed as the second argument, `plot_performance` will ask what label name to put in the plot for every performance point.
