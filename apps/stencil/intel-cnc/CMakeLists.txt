# to be included from $CNCROOT/samples/CMakeLists.txt

set(myname Stencil1D)

add_executable(${myname} ${myname}_main.cpp ${myname}_updateStep.cpp )
target_link_libraries(${myname} ${NEEDED_LIBS})

if(USE_ITAC)
  set_itac_link_flags(${myname})
endif(USE_ITAC)

unset(myname CACHE)
