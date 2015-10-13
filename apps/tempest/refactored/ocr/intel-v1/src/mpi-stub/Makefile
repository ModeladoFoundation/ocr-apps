#CC = icc -static
CC = gcc -static
#CXX = icpc -static
CXX = g++ -static
AR = ar rcu

RM = rm -f
MKDIRS = mkdir -p

LIBDIR = ./lib
SRCDIR = ./src
INCDIR = ./include

TARGET = $(LIBDIR)/libmpi.a

all : $(TARGET)

$(LIBDIR)/lib%.a: $(SRCDIR)/%.o
	-@$(MKDIRS) $(dir $@)
	$(AR) $@ $^

%.o : %.cpp
	-@$(MKDIRS) $(dir $@)
	$(CC) -I$(INCDIR) -c $^ -o $@

clean:
	$(RM) -f *~ */*~ $(SRCDIR)/*.o
	$(RM) -f $(TARGET)
