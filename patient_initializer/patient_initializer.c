#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <unistd.h>


#include <errno.h>
#include <string.h>
int errvalue;


#include "../patient/patient.h"

int main(int argc, char **argv)
{
    // arguments:
    // <num patients>
    // <heart vital rate sev>
    // <sys bp sev>
    // <sys ds sev>
    // <temp sev>
    // < resp sev>
    // < o2 sev>

    struct inheritance inherit;
    if (argc < 8)
    {
        printf("ERROR: Please pass in the amount of healthy patients followed by the amount of unhealthy patients\n");
        // return -1;
    }

    short patients = atoi(argv[1]);
    if (patients < 0)
    {
        printf("ERROR: Please pass in non-negative amount of patients\n");
        return -1;
    }

    for (short i = 0; i < argc; i++)
    {
        printf("argv[%d] is %s int %d\n", i, argv[i], atoi(argv[i]));
    }

    for (short i = 0; i < patients; i++)
    {
		printf("Generating patient %d\n", i + 1);
        for (short j = 1; j < argc; j++)
        {
            char* vitalType;

            int length = snprintf(NULL, 0, "%d", j);

            vitalType = malloc(length + 1);

            snprintf(vitalType,length+1, "%d", j);

            char *args[] = {"patient", vitalType, argv[j]};
            inherit.flags = 0;
            if ((spawn("patient", 0, NULL, &inherit, args, environ)) == -1)
            {
                printf("ERROR: Unable to create patient %d vital %d.\n", i + 1, j);
            	errvalue = errno; // preserve value as first printf may change errno
            	printf( "The error generated was %d\n", errvalue );
            	printf( "That means: %s\n", strerror( errvalue ) );
            }
        }
    }

    // patient takes vital type <enum 0-6> & severity <-3 to 3>
    return 0;
}
