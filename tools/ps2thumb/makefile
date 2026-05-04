# サムネイル用縮小プログラム
# 2001 / 5 / 1  Yuko Yano

thumbnail :thumbnail.o trans.o
	gcc -g -o $@  $^ -lm

%.o : %.c
	gcc -g -c -Wall $<  -o $@

clean:
	rm *.o
