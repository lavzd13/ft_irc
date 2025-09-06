# -------------------------------------------------------------------
# VARIABLES
# -------------------------------------------------------------------

# Source file directories
OBJDIR			= ./.objects/
SRCDIR  		= ./sources/

# Other directories
INCDIR			= ./includes/

# Source files
SOURCES			= main.cpp Server.cpp Client.cpp Utils.cpp Channel.cpp Commands.cpp

INCLUDES 		= Server.hpp Client.hpp Utils.hpp Responses.hpp Channel.hpp Colors.hpp

OBJ_FILES 		:= $(SOURCES:.cpp=.o)

OBJS			:= $(addprefix $(OBJDIR), $(OBJ_FILES))

# Compilation variables
NAME			= ircserv
CXX 			= c++
CXXFLAGS 		= -Wall -Werror -Wextra -std=c++98 -pedantic
RM 				= rm -rf
INCLUDES		:= -I$(INCDIR)

DEPFILES = $(OBJS:.o=.d)

# -------------------------------------------------------------------
# RULES
# -------------------------------------------------------------------

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) 

debug:
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -g -D DEBUG=1" re

clean:
	$(RM) $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re debug

# -------------------------------------------------------------------
# OBJECT RULES
# -------------------------------------------------------------------
# // These have to be implemented for each directory in the project

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< $(INCLUDES) -o $@

-include $(DEPFILES)