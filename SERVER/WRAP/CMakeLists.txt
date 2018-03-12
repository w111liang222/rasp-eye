add_library(WRAP Wrap.c)

install (TARGETS WRAP DESTINATION bin)
install (FILES Wrap.h DESTINATION include)
