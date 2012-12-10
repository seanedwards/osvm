
echo "Running 3 instances of programs/sem_blocker.pb"
echo "Output will continue indefinitely as 3 processes cycle through waiting/signaling semaphore 1."
echo "Press enter to begin the line noise."
read
./vm -d -t 20 programs/sem_blocker.pb programs/sem_blocker.pb programs/sem_blocker.pb

