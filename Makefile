# Name: Donald Tang, Tianwei
# I pledge my honor that I have abided by the Stevens Honor System. 

NAME = VMSimulator
SRC = VMSimulator.cpp
RM=rm -f

all:
	g++ $(SRC) -o $(NAME)

clean:
	-$(RM) *~
	-$(RM) *#*
	-$(RM) *swp
	-$(RM) *.core
	-$(RM) *.stackdump
	-$(RM) $(SRCPATH)*.o
	-$(RM) $(SRCPATH)*.obj
	-$(RM) $(SRCPATH)*~
	-$(RM) $(SRCPATH)*#*
	-$(RM) $(SRCPATH)*swp
	-$(RM) $(SRCPATH)*.core
	-$(RM) $(SRCPATH)*.stackdump
	-$(RM) $(NAME)

fclean: clean
	-$(RM) $(NAME)

re: fclean all

