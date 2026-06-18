CXX := clang++
TARGET := testmc
SOURCES := TestMC.cpp ../UtilitiesDJD/RNG/NormalGenerator.cpp

BOOST_INC ?= $(shell if [ -f /opt/homebrew/include/boost/random.hpp ]; then echo /opt/homebrew/include; elif [ -f /usr/local/include/boost/random.hpp ]; then echo /usr/local/include; fi)

CXXFLAGS := -std=c++17 -Wall -Wextra -I..
ifneq ($(BOOST_INC),)
CXXFLAGS += -I$(BOOST_INC)
endif

.PHONY: all run clean info

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	@if [ -n "$(STEPS)" ] && [ -n "$(SIMS)" ]; then \
		printf "$(STEPS)\n$(SIMS)\n" | ./$(TARGET); \
	else \
		./$(TARGET); \
	fi

clean:
	rm -f $(TARGET)

info:
	@echo "Compiler: $(CXX)"
	@echo "Boost include: $(if $(BOOST_INC),$(BOOST_INC),not found)"
	@echo "Build command: $(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)"
