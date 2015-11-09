CC=gcc
CFLAGS=-Wall
LIBS=-lssl

devel:
	$(CC) -o pushtest-devel pushtest.c Helper/SSLHelper.c Helper/RemoteNotification.c -DIS_DEVELOPMENT_VERSION $(LIBS)

release:
	$(CC) -o pushtest-release pushtest.c Helper/SSLHelper.c Helper/RemoteNotification.c $(LIBS)

clean:
	rm pushtest-devel
	rm pushtest-release
