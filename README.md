# compare
Caitlyn Romano - cer161
Anthony Romanushko - ar1738

Testing Strategey:
We determined our program was correct by reviewing the requirements and testing to ensure we met the criteria. We tested our program using numerous test cases, a few of which are listed below:

1.) Ensured our program works for all possible regular arguments
    - User enters a valid file / directory 
    - User enters an invalid file / directory which cannot be opened 
    - User enters a file that does not contain the specified extension
    - User enters a directory which contains files without the specified extension
    - User enters a directory with multiple nested directories
    - User enters numerous files / directories
    - User enters a directory which contains entries beginning with a period 
2.) Ensured our program works for all possible optional arguments
    - User enters optional arguments in any order
    - User enters an optional argument with an invalid value
    - User enters an invalid optional argument 
    - User enters an empty file name suffix 
3.) Ensured correct calculation of Word Frequency Distribution
    - capitals and punctuation are omitted when comparing word equivalence (except for hyphens)
    - frequencies sum to 1
    - program can handle words of all lengths (no maximum word length)
 4.) Ensured correct operation of threads
    - checked for failure creating thrads (terminate immediately)
    - ensured correct number of threads are created
 5.) Memory
    - ensured memory is dynamically allocated when necessary
    - tested for memory leaks using address sanitizer
