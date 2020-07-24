CC=g++
OBJS=main.o epd.o it8951_dev_usb.o usb_epd.o tool.o img_proc.o

all: main

%.o:%.cpp
	$(CC) -c -o $@ $<

main:$(OBJS)
	$(CC) -lusb-1.0 -lopencv_highgui -lopencv_core -lopencv_imgproc \
	-o main $^

clean:
	rm *.o
