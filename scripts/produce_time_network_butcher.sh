#!/bin/sh

python3 run.py -c example_configurations/butcher_test.ini -o output_butcher
python3 predict.py -c example_configurations/butcher_predict.ini -r output_butcher/LRRidge.pickle -o output_butcher_predict
