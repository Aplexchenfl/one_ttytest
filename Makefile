CC = arm-linux-gcc
TARGET1 = one_ttytest
SOURCE1 = one_ttytest.c

TARGET2 = one_ttytest_readorwrite
SOURCE2 = one_ttytest_readorwrite.c


all:
	$(CC)  $(SOURCE1)   -o  $(TARGET1)
	cp  $(TARGET1)  ../shell/  -rf
	$(CC)  $(SOURCE2)   -o  $(TARGET2)
	cp  $(TARGET2)  ../shell/  -rf

clean:
	rm ../shell/$(TARGET1) -rf
	rm $(TARGET1) -rf
	rm ../shell/$(TARGET2) -rf
	rm $(TARGET2) -rf
