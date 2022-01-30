#!/bin/bash

SCRIPT_DIR="$(dirname ${BASH_SOURCE[0]})"
cd $SCRIPT_DIR
PWD=$(pwd)

# Config
BIN="generate_report"
COMPLETIONS_PATH="/usr/share/bash-completion/completions/"

# Make executable script in project directory
echo -e "#!/bin/sh\npython3 $PWD/src/main.py \$@" > $BIN
chmod u+x $BIN

# Install bash completions
# python3 src/gen_completions.py
# echo "Need sudo to copy completions  (on line $LINENO)"
# sudo cp completions.sh $COMPLETIONS_PATH$BIN || echo "Warning: Completions not set"
# echo

echo "Done"
