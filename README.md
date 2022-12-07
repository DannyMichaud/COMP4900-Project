# COMP4900-Project

COMP 4900 project

## Note for running hospital system

i. No arguments specified - hospital system will automatically spawn 5 monitors
ii. "noSpawn" specified as first argument - hospital system will not automatically spawn any monitors

## How to run the project

1. Built the files in the project
2. Move the generated binary files to the VM
3. Create two terminal sessions (under Target Navigator, right click the VM and select `Start SSH Session` twice)
4. Run the hospital system program within the VM under one terminal
5. Run the monitor within the VM under the other terminal
6. Run the patient program using the momentics IDE, outside of the VM

## How to shutdown the project

1. Copy "shutdown" binary into VM
2. Run ./shutdown in VM
3. Wait a bit
4. Done!
