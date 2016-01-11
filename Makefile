CFLAGS 	= -fPIC  -g #-Wall
CC 	= gcc
LLFLAGS = $(CFLAGS) -shared

#********************************************************#
#FOR .C to .O
NO_COMMAND = $(CC) $(CFLAGS) -c $< 


LIB = -L/usr/lib/ #do not delete space before -> #
#********************************************************#
#FOR EXEC WITH LL LIB
LL_USE	=-lll 
COMMAND_LL = $(CC) $(CFLAGS) $< -o $@  ll.c #$(LIB) $(LL_USE)  


#********************************************************#
#FOR EXEC WITH TFS LIB
TFS_USE =-ltfs 
COMMAND_TFS = $(CC) $(CFLAGS) $< -o $@  tfs.c  ll.c #$(LIB)  $(LL_USE)  $(TFS_USE)

#********************************************************#
HEADERS = $(wildcard *.h)

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

FIRST = ll lllib tfs_create tfs_analyze tfs_partition 

SECOND =  tfs lltfs tfs_format tfs_mkdir tfs_rm
#path
all: $(FIRST) $(SECOND)

#********************************************************#
# PARTIE 1
ll:	ll.c
	$(NO_COMMAND)

lllib:	ll.o
	$(CC) $(LLFLAGS) $< -o libll.so
	#
	#
	#********************************************************#
	# Put the bibi in /usr/lib/ 
	#********************************************************#
	sudo rm -f /usr/lib/libll.so
	sudo cp libll.so /usr/lib/
	sudo rm libll.so
	#sudo rm ll.o
	#********************************************************#
	#
	#
tfs_create: tfs_create.c 
	$(COMMAND_LL)

tfs_analyze: tfs_analyze.c
	$(COMMAND_LL)

tfs_partition: tfs_partition.c
	$(COMMAND_LL)

#********************************************************#
# PARTIE 2

path:	path.c
	$(NO_COMMAND)

tfs:	tfs.c
	$(CC) $(CFLAGS) -c $< $(LIB) $(LL_USE)

lltfs:	tfs.o
	#path.o
	$(CC) $(LLFLAGS) $< -o libtfs.so $(LIB) $(LL_USE)
	#
	#
	#********************************************************#
	# Put the bibi in /usr/lib/
	#********************************************************#
	sudo rm  -f /usr/lib/libtfs.so
	sudo cp libtfs.so /usr/lib/
	sudo rm libtfs.so
	#sudo rm tfs.o
	#********************************************************#
	#
	#
tfs_format : tfs_format.c
	$(COMMAND_TFS)

tfs_mkdir : tfs_mkdir.c
	$(COMMAND_TFS)

tfs_rm : tfs_rm.c
	$(COMMAND_TFS)
#********************************************************#
# CLEAN

clean:
	rm -f $(OBJECTS)

mrproper: clean
	rm -f $(FIRST) $(SECOND)
	sudo rm -f /usr/lib/libtfs.so
	sudo rm -f /usr/lib/libll.so