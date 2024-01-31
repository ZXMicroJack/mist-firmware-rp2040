default: mkimage Bit2Bin mkuf2

mkimage: ./mkimage.c
	gcc $^ -o $@

Bit2Bin: ./Bit2Bin.c
	gcc $^ -o $@

mkuf2: ./mkuf2.c
	gcc $^ -o $@

clean:
	-rm Bit2Bin mkimage mkuf2	
