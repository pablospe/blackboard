#!/bin/bash

find /home/spe/projects/laviola_dataset/ -name "*.dat" | grep "[0-9].dat" | grep --invert-match "data/" | grep "training1" | sort -u >file_list_digits_training1.txt
find /home/spe/projects/laviola_dataset/ -name "*.dat" | grep "[0-9].dat" | grep --invert-match "data/" | grep "training2" | sort -u >file_list_digits_training2.txt
find /home/spe/projects/laviola_dataset/ -name "*.dat" | grep "[0-9].dat" | grep --invert-match "data/" | grep "testing" | sort -u >file_list_digits_testing.txt
find /home/spe/projects/laviola_dataset/ -name "*.dat" | grep "[0-9].dat" | grep  "data/" | sort -u >file_list_digits_data.txt
