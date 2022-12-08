#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <unistd.h>
#include <string.h>

#include "../patient/patient.h"

// arguments:
// <num patients>
// optional <patient vitalType arg>
// optional <patient sev arg>
// ...
// optional <patient vitalType arg>
// optional <patient sev arg>

// any patients not passed any args
// will be given a default vitalType & severity of 1

int main(int argc, char **argv)
{
    short numPatients;
    int status;
    pid_t pid;
    struct inheritance inherit;
    if (argc < 2)
    {
        printf("ERROR: Please pass in the amount of patients to initialize\n");
        return -1;
    }

    numPatients = atoi(argv[1]);
    if (numPatients < 0)
    {
        printf("ERROR: Please pass in non-negative amount of patients\n");
        return -1;
    }
    inherit.flags = 0;
    char *vitalType, *severity;
    int j = 2;
    for (short i = 0; i < numPatients; i++)
    {
        // increment j to move through the args
        vitalType = j < argc ? argv[j++] : "1";
        severity = j < argc ? argv[j++] : "1";
        printf("PATIENT INITIALIZER: Generating patient %d vitalType %s severity %s\n", i + 1, vitalType, severity);

        char *args[] = {"patient", vitalType, severity, NULL};
        if ((spawn("patient", 0, NULL, &inherit, args, environ)) == -1)
        {
            printf("ERROR: Unable to create patient %d\n", i + 1, j);
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
