ECC algorithm is implemented in the file ecc.cpp as a function with the name cvFindTransform.

The file image_registration is the main function that takes input arguments, calls ECC algorithm and displays/saves the results.

To build the app, you can compile the source files into an executable with name ‘ecc’ (you need to include the necessary headers and libs from opencv). 
Or, you can simply use cmake/make just and follow the common steps:

1) Open terminal window and go to the root directory that contains the source files. 
2) Run the following:
>> mkdir build
>> cd build
>> cmake .. 
>> make
>> ecc <input_arguments> (see below) 

Example of use (given that the name of the executable file is 'ecc'):

ecc -t -template.pgm -i image.pgm -o warp.ecc

or

ecc -t -template.pgm -i image.pgm -o warp.ecc -init init.txt -m homography -eps 0.0001 -n 60 -v 1

The first call contains only the necessary input arguments while the latter contains all possible flags.

-t		the image filename that is used as template 
-i		the image filename that is used as an input image that must be warped in order to give a new image close to the template
-o		the filename of the file where the transformation (warp) is saved in YAML format

OPTIONAL:
-init		the filename of the file that contains the warp initialization in YAML format. DEFAULT: identity transformation.
-m		the type of the transformation. It can be any of the strings {translation, euclidean, affine, homography}. DEFAULT: affine
-n		the number of iterations. DEFAULT: 50
-eps		termination tolerance. DEFAULT: 0.001
-v		display execution time and images after the alignment. DEFAULT: 0 	


This software is provided as is without any kind of warranty. The use and the redistribution of the code is only permitted for academic purposes

Author: Georgios Evangelidis
email: 	george.evangelidis@gmail.com
