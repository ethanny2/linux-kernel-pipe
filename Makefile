
obj-m +=  numpipe.o

all: producer consumer 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
producer:producer.c
	gcc -o producer producer.c

consumer:consumer.c
	gcc -o consumer consumer.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm consumer producer
