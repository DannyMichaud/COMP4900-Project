#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <unistd.h>
#include <string.h>

#include "../patient/patient.h"

// arguments:
// <num patients>
// <heart vital rate sev>
// <sys bp sev>
// <sys ds sev>
// <temp sev>
// < resp sev>
// < o2 sev>
int main(int argc, char **argv)
{
    int status;
    pid_t pid;
    struct inheritance inherit;
    if (argc < 8)
    {
        printf("ERROR: Please pass in the amount of patients, then the severity for each of their tracked vitals\n");
        return -1;
    }

    short patients = atoi(argv[1]);
    if (patients < 0)
    {
        printf("ERROR: Please pass in non-negative amount of patients\n");
        return -1;
    }

    inherit.flags = 0;
    for (short i = 0; i < patients; i++)
    {
        printf("PATIENT INITIALIZER: Generating patient %d\n", i + 1);
        for (short j = 1; j < argc; j++)
        {
            char *vitalType;

            int length = snprintf(NULL, 0, "%d", j);
            vitalType = malloc(length + 1);

            snprintf(vitalType, length + 1, "%d", j);

            char *args[] = {"patient", vitalType, argv[j]};
            if ((spawn("patient", 0, NULL, &inherit, args, environ)) == -1)
            {
                printf("ERROR: Unable to create patient %d vital %d.\n", i + 1, j);
            }
        }
    }

    while (1)
    {
        if ((pid = wait(&status)) == -1)
        {
            printf("PATIENT INITIALIZER: no more child processes\n");
            return 0;
        }
        printf("PATIENT INITIALIZER: a patient terminated, pid = %d\n", pid);

        if (WIFEXITED(status))
        {
            printf("PATIENT INITIALIZER: child terminated normally, exit status = %d\n",
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("PATIENT INITIALIZER: child terminated abnormally by signal = %X\n",
                   WTERMSIG(status));
        }
    }

    return 0;
}
