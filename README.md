# DiskAnalyzer

Created a daemon that analyzes the space used on a storage device starting from a given path, and build a utility
program that allows using this functionality from the command line.
The user can add, delete or display information about the various analysis tasks and for each analysis task, a separate
thread is launched that performs the analysis of the occupied disk space.
