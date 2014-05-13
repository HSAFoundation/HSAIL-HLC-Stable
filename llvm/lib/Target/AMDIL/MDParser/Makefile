CC     = clang++
CXXFLAGS = -Wall -g -O0 -DYYDEBUG=1 -DYYVERBOSE=1 -D__STDC_LIMIT_MACROS=1 -D__STDC_CONSTANT_MACROS=1
BIN    = ILParser

$(BIN) : AMDILMDParser.o AMDILMDScanner.o Main.cxx AMDILMDTypes.cpp AMDILDwarfParser.cpp
	$(CC) $(CXXFLAGS) -pthread Main.cxx AMDILMDScanner.o AMDILMDParser.o AMDILMDTypes.cpp AMDILDwarfParser.cpp -o $(BIN) -I-I. ../../../Support/build/lnx64a/B_dbg/LLVMSupport.a ../../../../../../contrib/STLport/lib/lnx64a/libstlport.a -ldl

AMDILMDParser.o : AMDILMDParser.y
	bison -t --verbose AMDILMDParser.y -o AMDILMDParser.tab.cpp
	$(CC) $(CXXFLAGS) -I. -c -o AMDILMDParser.o AMDILMDParser.tab.cpp

AMDILMDScanner.o : AMDILMDParser.y AMDILMDParser.l
	flex --nounistd -d -o lex.yy.cpp AMDILMDParser.l
	$(CC) $(CXXFLAGS) -I. -c -o AMDILMDScanner.o lex.yy.cpp

.PHONY : clean
clean :
# remove bison generated files
	rm -rf AMDILMDParser.tab.cpp AMDILMDParser.tab.hpp location.hh position.hh stack.hh
# remove flex generated files
	rm -rf lex.yy.cpp
# remove compiled files
	rm -rf $(BIN) *.o *~

