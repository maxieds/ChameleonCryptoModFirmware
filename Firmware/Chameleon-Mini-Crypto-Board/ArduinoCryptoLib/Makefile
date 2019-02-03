
.PHONY: all clean check

TOPDIR = 
SRCDIR = $(TOPDIR)/AESCrypto

LIBRARY = libAESCrypto.a

CXX=avr-g++
HOSTARCH_FLAGS=-mmcu=avrxmega7 -D__AVR_ATxmega128A4U__
CPPFLAGS = -I$(SRCDIR) -DHOST_BUILD 
CXXFLAGS = -g -Wall $(CPPFLAGS) $(HOSTARCH_FLAGS)

SOURCES = \
	AESCrypto/AES128.cpp \
	AESCrypto/AES192.cpp \
	AESCrypto/AES256.cpp \
	AESCrypto/AESCommon.cpp \
	AESCrypto/AVRGCCSupplements.cpp \
	AESCrypto/BlockCipher.cpp \
	AESCrypto/Crypto.cpp
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

default: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(RM) $(LIBRARY)
	$(AR) cr $(LIBRARY) $(OBJECTS)

clean:
	$(RM) -f $(OBJECTS) $(LIBRARY)

$(SRCDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

