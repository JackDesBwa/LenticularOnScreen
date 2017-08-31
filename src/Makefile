NaE_calib: NaE_test05b.o NeilandEliza_CV01.o
	${CXX} -o $@ $^ $(shell pkg-config opencv --libs) ${LDFLAGS}

clean:
	${RM} *.o NaE_calib
