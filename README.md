# COMP4900-Project

**MADE PUBLIC FOR MARKING PURPOSES (prof permission given). DO NOT USE THIS CODE ELSEWHERE!!!**

COMP 4900 project

## Note for running hospital system

1. No arguments specified - hospital system will automatically spawn 5 monitors
2. "noSpawn" specified as first argument - hospital system will not automatically spawn any monitors

## How to run the project

1. Built the files in the project
2. Move the generated binary files to the VM
3. Create two terminal sessions (under Target Navigator, right click the VM and select `Start SSH Session` twice)
4. Run the hospital system program within the VM under one terminal
5. Run the monitor within the VM under the other terminal
6. Run the patient program using the momentics IDE, outside of the VM

NOTE: If you're trying to run more than one patient process, replace step 6 with the following:

7. Run the patient_initializer program using the momentics IDE, outside the VM.
   - The first argument passed in is a number representing how many patient processes to run.
   - The following arguments are optional, but every argument passed in after will be passed into the generated patient process.
   - e.g. running `patient_initializer 3 2 2` will generate 3 patient processes, pass the arguments `2 2` into the first patient, then will default to passing in `1 1` for the other 2 patients.

## How to shutdown the project

1. Copy "shutdown" binary into VM
2. Run ./shutdown in VM
3. Wait a bit
4. Done!
