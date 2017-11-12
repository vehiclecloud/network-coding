#!/bin/bash -ev

find . -type f -name '*.cpp' -o -name "*.h"| xargs sed -i 's/HAGGLE_DBG2/printf/g'

find . -type f -name '*.cpp' -o -name "*.h"| xargs sed -i 's/HAGGLE_DBG/printf/g'

find . -type f -name '*.cpp' -o -name "*.h"| xargs sed -i 's/HAGGLE_ERR/printf/g'

