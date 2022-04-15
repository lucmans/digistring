`generate_report` compares the output of Digistring to annotations from a dataset and generates a performance result.


# Requirements
Python >=3.10 and Matplotlib >=3.5.1

On Arch Linux:  
`sudo pacman -S python3 python-matplotlib`


# Usage instructions (todo)
To generate performance report of Digistring on a dataset, run:  
`./generate_report <dataset name> <dataset annotations> <digistring results> <performance report output>`  
Valid dataset names are:  
fraunhofer MDB-stem-synth

To generate Bash (tab) completions for dataset names, run:  
`./generate_report generate`  
This generates the file `completions.sh`.  
Running `source completions.sh` adds the completions to the current shell.


# TODOs
Use note event duration information instead of only using next note event for note termination.  
Use NoteEvents being sorted for more efficient `is_monophonic()`, `is_polyphonic()` and time based getters.
