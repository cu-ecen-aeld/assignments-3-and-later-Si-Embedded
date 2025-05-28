# To do

* [x] Modify to spawn a thread with each new connection.
* [x] Synchronise writes to /var/tmp/aesdsocketdata.
* [x] Thread should exit when the connection breaks.
* [x] Wait for all threads upon SIGTERM/SIGINT.
* [x] Use a singly linked list to manage threads.
* [x] Use pthread_join () to join completed threads.
* [x] Write timestamp:time every 10 seconds to /var/tmp/aesdsocketdata.
* [x] Return full /var/tmp/aesdsocketdata upon receiving data.