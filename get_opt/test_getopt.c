#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>

/* The option struct in getopt.h:
    struct option {
        char *name; // --input
        int has_arg; // no_argument, optional_argument, required_argument
        int *flag;  // usually is NULL
        int val;
    };
    
example:
    {"input", required_argument, NULL, 'i'},
*/

/*
 * Reference: https://www.ibm.com/developerworks/cn/aix/library/au-unix-getopt.html.
 * Some usefull val:
 *      no_argument : 0 : do not need an argument;
 *      required_argument: 1 : require an argument;
 *      optional_argument: 2 : argument is optional;
 *      
 *      optind: 
 *      optarg:
 *      optopt:
 * */

void show_usage(const char *argv0)
{
    printf("Demonstarte the use of getopt.\n");
    printf("Usage: %s [options]\n"
            "Options:\n"
            "\t-h,--help\tShow this help message\n"
            "\t-i,--input\tSelect input file\n"
            "\t-o,--output\tSet output file name\n"
            "\t-t,--time\tSet input time\n",argv0);

    printf("Try this: %s %s\n", argv0, "-i f1 -o f2 -t 3");
    printf("Or try this: %s %s\n", argv0, "-i f1 f2 f3 -o f4 -t 3");
    printf("Finally try this: %s %s\n", argv0, "-i f1 -i f2 -i f3 -o f4 -t 3");
    exit(-1);
}

int main(int argc, char *argv[])
{
    int longIndex = 0;
    int time, numInputFiles, cnt = 0;
    char *output = NULL;
    char *input = NULL;
    char **inputFiles = NULL;
    char inputs[100][200];
    
    static const char *optString = "hi:o:t:";
    /* Prepare for option struct */
    static const struct option longOpts[] = {
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {"time", required_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
    };
    
    int opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while(1) {
        if( opt < 0 ) break;
        //printf("longIndex = %d\n", longIndex);
        switch( opt ) {
            case 'i':
                input = optarg;
                strcpy(inputs[cnt++], input);
                break;
            case 'o':
                output = optarg;
                break;
            case 't':
                time = atof(optarg);
                break;
            case 'h':
                show_usage(argv[0]);
                break;
            default:
                break;
        }
        opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
        printf("input = %s; output = %s\n", input, output);
        //if(input != NULL) printf("input = %s\n", input);
        //if(output != NULL)printf("output = %s\n", output);

    }

    printf("What the function really get:\n");
    for(int i = 0; i < argc; i ++) printf("\t%s", argv[i]);
    printf("\nWhat the optget can get:\n");
    inputFiles = argv + optind;
    numInputFiles = argc - optind;
    printf("\toptind = %d; argc = %d; numInputFiles = %d\n", optind, argc, numInputFiles);
    for(int i = 0; i < numInputFiles; i ++) printf("\t%s", inputFiles[i]);
    printf("\nChange the --input into no_argument can help.\n");

    printf("\nThe really inputFile is:\n");
    for(int i = 0; i < cnt; i ++) printf("\t%s",inputs[i]);
    printf("\n");
    return 0;
}
