SRCS1 := mp3_tag_main.c mp3_tag_fun.c
TRGT1 := mp3_tag_reader

${TRGT1} : ${SRCS1}
	gcc $^ -o $@

clean :
	rm -f ${TRGT1}
