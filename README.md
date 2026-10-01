# Fall2026_EECS348_Assignment_3


Hello again grader,

Hope your October is nice and not too chilly and rainy. Attached is a similar structure to Assignment 2, with recursive grading in feedback. The project structure is as follows:

* Frias_Lucas_Assignment_3
	* ANALYSIS.md
		* the main analysis, written in markdown format
	* ANALYSIS.pdf
		* the main analysis, saved as a common adobe pdf format
	* exec
		* arm64-osx27
			* anthropic - apple silicon executable for macos27
			* moonshot - apple silicon executable for maco27
			* DEMO.txt - a copy demo input
		*x86-osx15
			*anthropic -x86 executable for macos15
			*moonshot -x86 executable for macos15
			* DEMO.txt - a copy demo input
		*x86-KUdebian
			*anthropic- x86 executable for KU's ubuntu debian
			*moonshot - x86 executable for KU's ubuntu debian
			*human - x86 executable for KU's ubuntu debian
			*generator.py - resource used to generate test cases
			* inputs - directory with text files with N number 
			of inputs denoted by their filename, used for testing
			* timings.csv - csv file with all times compared
			during the execution
			* DEMO.txt - demo text input file
	* prompt_rubric
		* PROMPT.md
			* the exact prompt given to both models
	* feedback
		* ADVICE_1.md
			* the initial grading given by the LLM
	* src
		* anthropic.cpp
			* Sonnet's output, saved as its company's name
		* moonshot.cpp
			* KIMI's output, saved as its company's name
		* human.cpp
			* My modified output of Sonnet using Opus 5.5
