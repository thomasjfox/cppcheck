#!/usr/bin/python
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2014 Daniel Marjamaeki and Cppcheck team.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Draw a diagram of the AST (abstract syntax tree) using graphviz.
You obtain the input data by running cppcheck --dumb.
"""

import cppcheckdata
import subprocess
import argparse
import time
import sys

# Settings
BIN_DOT = 'dot'

# Argument handling
parser = argparse.ArgumentParser(
    description="Draw a diagram of the AST using graphviz's dot")
parser.add_argument('FILENAME', help="Input filename")
parser.add_argument('--output-format', default='png', required=False, help='dot output format. Can be ps2, png and others')
parser.add_argument('--node-limit', type=int, default=500, required=False, help='AST node  limit')

args = parser.parse_args()

INPUTFILE = args.FILENAME
OUTPUTGV = INPUTFILE + '.gv'
OUTPUTFINAL = INPUTFILE + '.' + str(args.output_format)
OUTPUTFORMAT = args.output_format

# Parse XML data
data = cppcheckdata.CppcheckData(INPUTFILE)

# Generate AST graph output
graph = 'digraph AST {\n'

def add_label(token):
    global graph

    escaped_label = token.str.replace('"', '\\"')
    graph += '    "' + token.Id + '" [label="' + escaped_label + '"];\n'

def add_node(token, astId, astToken):
        global graph

        if astId != astToken.Id:
            raise ValueError('AST tree error: ast id ' + astId + ' does not match id of ast token ' + astToken.Id)

        # Graph connection
        graph += '    "' + token.Id + '" -> "' + astId + '";\n'

count = 0
for token in data.tokenlist:
    if count > args.node_limit:
        print('WARNING: Note limit of ' + str(args.node_limit) + ' reached. Use --node-limit to increase. Skipping remaining nodes.')
        break

    if token.astOperand1Id or token.astOperand2Id:
        add_label(token)
        count += 1

    if token.astOperand1Id:
        add_label(token.astOperand1)
        add_node(token, token.astOperand1Id, token.astOperand1)
        count += 1

    if token.astOperand2Id:
        add_label(token.astOperand2)
        add_node(token, token.astOperand2Id, token.astOperand2)
        count += 1

graph += '}\n'

# Output to file
f = open(OUTPUTGV, 'w')
f.write(graph)
f.close()

try:
    start_sec = time.time()

    rtn = subprocess.call([BIN_DOT, OUTPUTGV,
                        '-T', OUTPUTFORMAT,
                        '-o', OUTPUTFINAL])
except:
    print("WARNING: Failure to run '" + BIN_DOT + "' executable.")
    print("You can still examine the output in '" + OUTPUTGV + "'")
    sys.exit(-1)

print('Dot call took ' + str(time.time() - start_sec) + ' seconds')
print('')
print('Wrote output do: ' + OUTPUTFINAL)
sys.exit(rtn)
