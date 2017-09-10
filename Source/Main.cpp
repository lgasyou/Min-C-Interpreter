#include "Stable.h"
#include "Script.h"
#include "Utils/FileReader.h"
#include "Utils/UnitTest.h"

int main(int argc, char *argv[]) {
#if UNIT_TEST == 1
	UnitTest::RunAndDisposeTests();

#elif DEBUG == 1
	FileReader reader{ "TestCases/main_function.test" };
	std::string source = reader.readAll();
	Script script = Script::Compile(source);
	script.Run();

#else
	if (argc == 1) {
		return 0;
	}
	const char *fileName = argv[1];
	DBG_PRINT << "fileName: " << fileName << '\n';

	FileReader reader{ fileName };
	std::string source = reader.readAll();
	DBG_PRINT << "source:\n" << source;

	Script script = Script::Compile(source);
	script.Run();

#endif // UNIT_TEST == 1
	system("pause");
	std::cout << "回收垃圾中请稍等";
	return 0;
}
