CC = gcc
LIBS = -lrabbitmq -lpthread -luuid

all: clean control_node computing_node

control_node: control_node.c
	$(CC) -o control_node.out control_node.c $(LIBS)

computing_node: computing_node.c
	$(CC) -o computing_node.out computing_node.c $(LIBS)

clean:
	rm -f *.txt
	rm -f *.out

run_server:
	sudo systemctl enable rabbitmq-server
	sudo systemctl start rabbitmq-server