# Build script for Linus systems

gcc -c -Wall -Werror -fpic Sockets.c
gcc -shared -o libsocket.so Sockets.o
sudo cp libsocket.so /usr/lib