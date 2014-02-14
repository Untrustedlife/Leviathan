
// Put your includes here //

// Also include file based on
#include "ProgramIncludes.h"

// I recommend using your namespace here
using namespace ;

// ------------------ ProgramConfiguration ------------------ //

#define PROGRAMCLASSNAME				class
#define PROGRAMNETWORKINGNAME			class
#define PROGRAMLOG						L"ProgramLogPrexif"
#define ENGINECONFIGURATION				L"./EngineConf.conf"
#define PROGRAMCONFIGURATION			L"./Program.conf"
#define PROGRAMKEYCONFIGURATION			L"./ProgramKeys.conf"
#define PROGRAMCHECKCONFIGFUNCNAME		class::CheckGameConfigurationVariables
#define PROGRAMCHECKKEYCONFIGFUNCNAME	class::CheckGameKeyConfigVariables
#define PROGRAMMASTERSERVERINFO			MasterServerInformation(L"ProgramMasters.txt", L"Program_" GAME_VERSIONS, L"http://mywebsite.com/", L"/MastersList.php", L"ProgramCrecentials.txt", false)
#define WINDOWTITLEGENFUNCTION			class::GenerateWindowTitle()

#define USERREADABLEIDENTIFICATION		L"Game client|server|master version " GAME_VERSIONS
#define GAMENAMEIDENTIFICATION			L"Program"
#define GAMEVERSIONIDENTIFICATION		GAME_VERSIONS

// Don't look at the mess ahead, just set the previous things and customize using virtual functions //

#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

#else
int main(int argcount, char* args[]){
#endif
	int Return = 0;
#ifdef _WIN32
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if(SUCCEEDED(CoInitialize(NULL))){
#else

#endif

#ifdef LEVIATHAN_USES_VLD
	// now that we are in code we can start tracking //

	VLDEnable();
#endif // LEVIATHAN_USES_VLD

	// create program object //
	PROGRAMCLASSNAME app;
	PROGRAMNETWORKINGNAME network;

	unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine(PROGRAMLOG, ENGINECONFIGURATION, PROGRAMCONFIGURATION, PROGRAMKEYCONFIGURATION, 
		&PROGRAMCHECKCONFIGFUNCNAME, &PROGRAMCHECKKEYCONFIGFUNCNAME));
	// customize values //
#ifdef _WIN32
	ProgramDefinition->SetHInstance(hInstance);
#endif
	ProgramDefinition->SetMasterServerParameters(PROGRAMMASTERSERVERINFO).SetPacketHandler(&network).SetApplicationIdentification(
		USERREADABLEIDENTIFICATION, GAMENAMEIDENTIFICATION, GAMEVERSIONIDENTIFICATION);

	// create window last //
	ProgramDefinition->StoreWindowDetails(WINDOWTITLEGENFUNCTION, true,
#ifdef _WIN32
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
#endif
		&app);

#ifdef _WIN32
	app.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));
#else
	wstring commandline = L"";
	for(int i = 1; i < argcount; i++){
		commandline += L" "+Leviathan::Convert::StringToWstring(args[i]);
	}
	app.PassCommandLine(commandline);
#endif

	if(app.Initialize(ProgramDefinition.get())){

		// this is where the game should customize the engine //
		app.CustomizeEnginePostLoad();

		// After everything is ready the command line should be flushed //
		app.FlushCommandLine();


		Logger::Get()->Info(L"Engine successfully initialized", true);
		Return = app.RunMessageLoop();
	} else {
		Logger::Get()->Error(L"App init failed, closing", true);
		app.Release();
		Return = 5;
	}
#ifdef _WIN32
}
//_CrtDumpMemoryLeaks();
CoUninitialize();
#endif

return Return;
}