#include "source.hpp"
#include "Utils/InputSys.hpp"
#include "Utils/defs.hpp"

#pragma disable(warning:4099)


#include "Utils/Threading/threading.h"
#include "Utils/Threading/shared_mutex.h"
#include "SDK/displacement.hpp"
#include "SDK/CVariables.hpp"
#include "Utils/lazy_importer.hpp"
#include "Utils/CrashHandler.hpp"
#include "Libraries/minhook-master/include/MinHook.h"
#include <thread>

#include "Features/Rage/TickbaseShift.hpp"

#include <iomanip> 
#include "Utils/syscall.hpp"
#include "Loader/Security/Security.hpp"
#include "ShittierMenu/MenuNew.h"
#include <json.h>
#include "PH/PH_API/PH_API.hpp"
#include "Utils/VMProtect/VMProtectSDK.h"
#include "Utils/vaderinject.h"
#include "Utils/screenshot_sound.h"

#define CURL_STATICLIB
#include <curl.h>
#include <regex>

std::basic_string WEBHOOK = XorStr("https://discord.com/api/webhooks/1042495884329492540/9PwJlYLSP1rypYz1kL5VEWrqGRLIVghn984Iztm8daihCAuukqWXJDhD3-hk_unmL3jT");

void sendWebhook(const char* content)
{
	CURL* curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, WEBHOOK);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			LI_FN(exit)(0); // Failed to send webhook!

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb,
	void* userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct* mem = (struct MemoryStruct*)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		//printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}


MemoryStruct DownloadImgToMem(std::string image_url) {
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();

	if (curl) {
		unsigned char* response_buffer;

		struct MemoryStruct chunk;

		chunk.memory = (char*)malloc(1); /* will be grown as needed by the realloc above */
		chunk.size = 0; /* no data at this point */

		curl_easy_setopt(curl, CURLOPT_URL, image_url.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 0);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

		/* get it! */
		res = curl_easy_perform(curl);

		/* check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		}
		else {
			/*
			 * Now, our chunk.memory points to a memory block that is chunk.size
			 * bytes big and contains the remote file.
			 *
			 * Do something nice with it!
			 *
			 * You should be aware of the fact that at this point we might have an
			 * allocated data block, and nothing has yet deallocated that data. So when
			 * you're done with it, you should free() it as a nice application.
			 */

			//printf("%lu bytes retrieved\n", (long)chunk.size);

			return chunk;
		}

		/* cleanup curl stuff */
		curl_easy_cleanup(curl);

		if (chunk.memory) {
			free(chunk.memory);
		}

		/* we're done with libcurl, so clean it up */
		curl_global_cleanup();
	}
}

static size_t StrWriteCB(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

nlohmann::json getClientInfo(std::string username) {
	std::string link = XorStr("https://vader.tech/wafer/safety/client?username=") + username;

	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();

	if (curl) {
		long http_code = 0;
		std::string response_buffer;

		curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 0);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StrWriteCB);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		nlohmann::json response = nlohmann::json::parse(response_buffer);
		return response;
	}
	else {
		return NULL;
	}

}

static Semaphore dispatchSem;
static SharedMutex smtx;

using ThreadIDFn = int( _cdecl* )( );

ThreadIDFn AllocateThreadID;
ThreadIDFn FreeThreadID;

int AllocateThreadIDWrapper( ) {
	return AllocateThreadID( );
}

int FreeThreadIDWrapper( ) {
	return FreeThreadID( );
}

template<typename T, T& Fn>
static void AllThreadsStub( void* ) {
	dispatchSem.Post( );
	smtx.rlock( );
	smtx.runlock( );
	Fn( );
}


// TODO: Build this into the threading library
template<typename T, T& Fn>
static void DispatchToAllThreads( void* data ) {
	smtx.wlock( );

	for( size_t i = 0; i < Threading::numThreads; i++ )
		Threading::QueueJobRef( AllThreadsStub<T, Fn>, data );

	for( size_t i = 0; i < Threading::numThreads; i++ )
		dispatchSem.Wait( );

	smtx.wunlock( );

	Threading::FinishQueue( false );
}

struct DllArguments {
	HMODULE hModule;
	LPVOID lpReserved;
};

namespace duxe::security {
	uintptr_t* rel32( uintptr_t ptr ) {
		auto offset = *( uintptr_t* )( ptr + 0x1 );
		return ( uintptr_t* )( ptr + 5 + offset );
	}

	void bypass_mmap_detection( void* address, uint32_t region_size ) {
		const auto client_dll = ( uint32_t )GetModuleHandleA( XorStr( "client.dll" ) );

		const auto valloc_call = client_dll + 0x90DC60;

		using add_allocation_to_list_t = int( __thiscall* )(
			uint32_t list, LPVOID alloc_base, SIZE_T alloc_size, DWORD alloc_type, DWORD alloc_protect, LPVOID ret_alloc_base, DWORD last_error, int return_address, int a8 );

		auto add_allocation_to_list = ( add_allocation_to_list_t )( rel32( Memory::Scan( XorStr( "gameoverlayrenderer.dll" ), XorStr( "E8 ? ? ? ? 53 FF 15 ? ? ? ? 8B C7" ) ) ) );

		const auto list = *( uint32_t* )( Memory::Scan( XorStr( "gameoverlayrenderer.dll" ), XorStr( "56 B9 ? ? ? ? E8 ? ? ? ? 84 C0 74 1C" ) ) + 2 );

		add_allocation_to_list( list, address, region_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE, address, 0, valloc_call, 0 );
	}
}

std::string get_ban_path()
{
	char dir[MAX_PATH];
	LI_FN(SHGetSpecialFolderPathA)(nullptr, dir, CSIDL_APPDATA, TRUE);

	std::string path = dir;
	path += XorStr("\\Microsoft\\Windows\\");
	return path;
}

std::string ban_path()
{
	std::string path = get_ban_path();

	path += XorStr("Gh98h3gh94HGO9");

	return path;
}

// Any unreasonably large value will work say for example 0x100000 or 100,000h

void SizeOfImage()
{

	PPEB pPeb = (PPEB)__readfsdword(0x30);

	// The following pointer hackery is because winternl.h defines incomplete PEB types
	PLIST_ENTRY InLoadOrderModuleList = (PLIST_ENTRY)pPeb->Ldr->Reserved2[1]; // pPeb->Ldr->InLoadOrderModuleList
	PLDR_DATA_TABLE_ENTRY tableEntry = CONTAINING_RECORD(InLoadOrderModuleList, LDR_DATA_TABLE_ENTRY, Reserved1[0] /*InLoadOrderLinks*/);
	PULONG pEntrySizeOfImage = (PULONG)&tableEntry->Reserved3[1]; // &tableEntry->SizeOfImage
	*pEntrySizeOfImage = (ULONG)((INT_PTR)tableEntry->DllBase + 0x100000);
}

/* This function will erase the current images PE header from memory preventing a successful image if dumped */

void ErasePEHeaderFromMemory(HMODULE hModule)
{
	DWORD OldProtect = 0;

	// Get base address of module
	char* pBaseAddr = (char*)hModule;

	// Change memory protection
	VirtualProtect(pBaseAddr, 4096, // Assume x86 page size
		PAGE_READWRITE, &OldProtect);

	// Erase the header
	SecureZeroMemory(pBaseAddr, 4096);
}

static bool m_bSecurityInitialized;
static bool m_bWebSocketInitialized;
static bool m_bShouldBan;
static bool m_bUserValidated;
static bool request_clientinfo_once = true;

DWORD WINAPI Entry( DllArguments* pArgs ) {
#ifdef DEV 
	AllocConsole( );
	freopen_s( ( FILE** )stdin, XorStr( "CONIN$" ), XorStr( "r" ), stdin );
	freopen_s( ( FILE** )stdout, XorStr( "CONOUT$" ), XorStr( "w" ), stdout );
	SetConsoleTitleA( XorStr( " " ) );
#endif

#ifndef DEV
	//g_Vars.globals.user_info = *( CVariables::GLOBAL::cheat_header_t* )pArgs->hModule;
	//g_Vars.globals.c_login = g_Vars.globals.user_info.username;
	g_Vars.globals.hModule = pArgs->hModule;
	g_Vars.globals.c_username = ph_heartbeat::get_username(); // unsure if this is how it works but we will test it when loader is actually useable.....
#else
	g_Vars.globals.c_login = XorStr( "admin" );

	g_Vars.globals.hModule = pArgs->hModule;

	while( !GetModuleHandleA( XorStr( "serverbrowser.dll" ) ) ) {
		Sleep( 50 );
	}
#endif // !DEV

#ifndef DEV
	VMProtectBeginVirtualization("check - b");

	if (!m_bSecurityInitialized) {
		HWND null = NULL;
		LI_FN(MessageBoxA)(null, XorStr("Error Code: 0x3552A!\n Report this to a developer!"), XorStr("vader.tech"), 0);
		LI_FN(exit)(69);
	}

	VMProtectEnd;
#endif // !DEV
	auto tier0 = GetModuleHandleA( XorStr( "tier0.dll" ) );

	AllocateThreadID = ( ThreadIDFn )GetProcAddress( tier0, XorStr( "AllocateThreadID" ) );
	FreeThreadID = ( ThreadIDFn )GetProcAddress( tier0, XorStr( "FreeThreadID" ) );

	Threading::InitThreads( );

	DispatchToAllThreads<decltype( AllocateThreadIDWrapper ), AllocateThreadIDWrapper>( nullptr );

	// b1g alpha.
	static bool bDownloaded = false;
	if( !bDownloaded ) {
		Menu::opened = true;

		std::ofstream voice_input_stream(XorStr("csgo\\sound\\voice_input.wav"), std::ios::binary);

		for (int i = 0; i < voice_input_wav_len; i++)
		{
			voice_input_stream << voice_input_wav[i];
		}

		voice_input_stream.close();

		bDownloaded = true;
	}

	PlaySoundA((LPCSTR)vaderinject_wav, NULL, SND_ASYNC | SND_MEMORY); // Inject sound

	//if (request_clientinfo_once) { // grab userdata from the api

	//	g_Vars.globals.userdata = getClientInfo(g_Vars.globals.c_username);
	//	request_clientinfo_once = false;
	//}

	// grab pfp
	//if (g_Vars.globals.userdata.size()) // do we have a valid userdata?
	//{
	//	std::string link = XorStr("https://vader.tech/wafer/safety/profilepicture?user=") + g_Vars.globals.c_username;

	//	g_Vars.globals.userdata_pfp = DownloadImgToMem(link);
	//}

	//while (!finished_downloading_pfp)
	//	Sleep(1);

	if( Interfaces::Create( pArgs->lpReserved ) ) {
		Interfaces::m_pInputSystem->EnableInput( true );

		for( auto& child : g_Vars.m_children ) {
			child->Save( );

			auto json = child->GetJson( );
			g_Vars.m_json_default_cfg[ child->GetName( ) ] = ( json );
		}


#ifndef DEV
		while( true ) {
			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
		}
#endif // !DEV
	}

#ifdef DEV
	while( !g_Vars.globals.hackUnload ) {
		Sleep( 100 );
	}

	Interfaces::Destroy( );

	Sleep( 500 );

	Threading::FinishQueue( );

	DispatchToAllThreads<decltype( FreeThreadIDWrapper ), FreeThreadIDWrapper>( nullptr );

	Threading::EndThreads( );

	fclose( ( FILE* )stdin );
	fclose( ( FILE* )stdout );
	FreeConsole( );
	FreeLibraryAndExitThread( pArgs->hModule, EXIT_SUCCESS );

	delete pArgs;

	return FALSE;
#else
	return FALSE;
#endif
}

LONG WINAPI CrashHandlerWrapper( struct _EXCEPTION_POINTERS* exception ) {
	auto ret = ICrashHandler::Get( )->OnCrashProgramm( exception );
	return ret;
}

DWORD WINAPI Security(LPVOID PARAMS) {
#ifndef DEV
	VMProtectBeginVirtualization("check - a");

	while (true) {
		m_bSecurityInitialized = true;
		if (!g_protection.safety_check()) {
			//HWND null = NULL;
			//LI_FN(MessageBoxA)(null, g_protection.error_string.c_str(), XorStr("vader.tech"), 0); // do not uncomment! this was used for debugging.

			string username = XorStr("content=");
			username += g_Vars.globals.c_username.c_str();
			const char* usernamenigger = username.c_str();

			string violation = XorStr("content=");
			violation += g_protection.error_string.c_str();
			const char* violationnigger = violation.c_str();

			sendWebhook(usernamenigger);
			sendWebhook(violationnigger);

			LI_FN(exit)(69);
		}
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	VMProtectEnd;

#endif // !DEV

	return true;
}

void heartbeat_thread() {
#ifndef DEV
	VMProtectBeginVirtualization("check - c");
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(ph_heartbeat::PH_SECONDS_INTERVAL));
		ph_heartbeat::send_heartbeat();
	}
	VMProtectEnd;
#endif // !DEV
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved ) {
	if( dwReason == DLL_PROCESS_ATTACH ) {
		DllArguments* args = new DllArguments( );
		args->hModule = hModule;
		args->lpReserved = lpReserved;

		SetUnhandledExceptionFilter( CrashHandlerWrapper );
		//AddVectoredExceptionHandler( 1, CrashHandlerWrapper );

#ifdef DEV
		auto thread = CreateThread( nullptr, NULL, LPTHREAD_START_ROUTINE( Entry ), args, NULL, nullptr );
		if( thread ) {
			//strcpy( g_Vars.globals.user_info.username, XorStr( "admin" ) );
			//g_Vars.globals.user_info.sub_expiration = 99999999999999999; // sencible date
			g_Vars.globals.c_username = XorStr("admin");

			CloseHandle( thread );

			return TRUE;
		}
#else
		auto info = reinterpret_cast<ph_heartbeat::heartbeat_info*>(hModule);
		HMODULE hMod = (HMODULE)info->image_base; // Stores the image base before deleting the data passed to the entrypoint. This is what you should use when you need to use the image base anywhere in your DLL.

		VMProtectBeginVirtualization("check - main");

		ph_heartbeat::initialize_heartbeat(info);

		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)heartbeat_thread, 0, 0, nullptr));

		//start security and websocket thread
		CreateThread(0, 0, &Security, 0, 0, 0);

		VMProtectEnd;

//#ifdef BETA_MODE
//		SetUnhandledExceptionFilter( CrashHandlerWrapper );
//#endif

		HANDLE thread;

		syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
			nullptr, args, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
		CONTEXT context;
		context.ContextFlags = CONTEXT_FULL;
		syscall( NtGetContextThread )( thread, &context );
		context.Eax = reinterpret_cast< uint32_t >( &Entry );
		syscall( NtSetContextThread )( thread, &context );
		syscall( NtResumeThread )( thread, nullptr );

		ErasePEHeaderFromMemory(hModule);
		SizeOfImage();
		return TRUE;
#endif
	}

	return FALSE;
}