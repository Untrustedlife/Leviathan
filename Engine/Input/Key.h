#ifndef LEVIATHAN_KEY
#define LEVIATHAN_KEY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
#include "Window.h"
#include "Utility/Iterators/WstringIterator.h"
#include "Common/StringOperations.h"

enum KEYSPECIAL {
	KEYSPECIAL_SHIFT = 0x1,
	KEYSPECIAL_ALT = 0x2,
	KEYSPECIAL_CTRL = 0x4,
	KEYSPECIAL_WIN = 0x8,
	KEYSPECIAL_SCROLL = 0x10,
	KEYSPECIAL_CAPS = 0x20
	//0x40
	//0x80 // first byte full
	//0x100
	//0x200
	//0x400
	//0x800
	//0x1000
	//0x2000
	//0x4000
	//0x8000 // second byte full (int range might stop here(
	//0x10000
	//0x20000
	//0x40000
	//0x80000
	//0x100000
	//0x200000
	//0x400000
	//0x800000 // third byte full
	//0x1000000
	//0x2000000
	//0x4000000
	//0x8000000
	//0x10000000
	//0x20000000
	//0x40000000
	//0x80000000 // fourth byte full (will need QWORD here)
	//0x100000000
};

namespace Leviathan{

	// base class to have pointers and house the lookup table for ease of use //
	class BaseKey{
	public:
		virtual inline ~BaseKey(){
		}

	protected:
		//static map<int, wstring> KeySpecialStringRepresentationMap;
	};


	template<class T>
	class Key : public BaseKey{
	public:
		DLLEXPORT inline Key<T>(){
			Extras = 0;
			Character = (T)0;
		}
		DLLEXPORT inline Key<T>(const T &character, const short &additional){
			Extras = additional;
			Character = character;
		}
		DLLEXPORT inline ~Key(){
		}

		DLLEXPORT inline bool Match(const Key<T> &other, bool strict = false) const{
			if(other.Character != this->Character)
				return false;
			if(strict){
				if(other.Extras != this->Extras)
					return false;
			} else {

				if((Extras & KEYSPECIAL_SHIFT) && !(other.Extras & KEYSPECIAL_SHIFT)){
					return false;
				}
				if((Extras & KEYSPECIAL_ALT) && !(other.Extras & KEYSPECIAL_ALT)){
					return false;
				}
				if((Extras & KEYSPECIAL_CTRL) && !(other.Extras & KEYSPECIAL_CTRL)){
					return false;
				}
			}
			return true;
		}

		DLLEXPORT inline wstring GenerateWstringMessage(const int &style = 0){
			// create a string that represents this key //
			if(style == 0){
				// debug string //
				wstring resultstr = L"Key["+Convert::ToWstring<wchar_t>((wchar_t)Character)+L"]=";

				resultstr += Convert::ToWstring<T>(Character);
				resultstr += L"(0x"+Convert::ToHexadecimalWstring<T>(Character)+L")+";

				return resultstr;
			}
			// various styles from which a key can be easily parsed //
			DEBUG_BREAK;
			return L"error";
		}

		DLLEXPORT bool Match(const T &chara, const short &additional, bool strict = false) const{
			return Match(Key<T>(chara, additional), strict);
		}

		DLLEXPORT bool Match(const T &chara) const{
			if(chara != this->Character)
				return false;
			return true;
		}


		DLLEXPORT T GetCharacter() const{
			return Character;
		}
		DLLEXPORT short GetAdditional() const{
			return Extras;
		}

		DLLEXPORT void Set(const T &character, const short &additional){
			Character = character;
			Extras = additional;
		}

		DLLEXPORT void SetAdditional(const short &additional){

			Extras = additional;
		}

		DLLEXPORT static void DeConstructSpecial(short keyspes, bool &Shift, bool &Alt, bool &Ctrl){

			if(keyspes & KEYSPECIAL_SHIFT)
				Shift = true;
			if(keyspes & KEYSPECIAL_ALT)
				Alt = true;
			if(keyspes & KEYSPECIAL_CTRL)
				Ctrl = true;
		}

		DLLEXPORT static Key<T> GenerateKeyFromString(const wstring &representation){
			if(representation.size() == 0){
				// empty, nothing to do //
				return Key<T>((T)0, 0);
			}
			StringIterator itr(representation);

			wstring converted;

			unique_ptr<wstring> str = itr.GetUntilNextCharacterOrAll(L'+');

			Convert::ToCapital(*str, converted);

			T character = Leviathan::Window::ConvertWstringToOISKeyCode(converted);
			short special = 0;

			while((str = itr.GetUntilNextCharacterOrAll(L'+'))->size() > 0){

				if(StringOperations::CompareInsensitive(*str, wstring(L"alt"))){
					special |= KEYSPECIAL_ALT;
				}
				if(StringOperations::CompareInsensitive(*str, wstring(L"shift"))){
					special |= KEYSPECIAL_SHIFT;
				}
				if(StringOperations::CompareInsensitive(*str, wstring(L"ctrl"))){
					special |= KEYSPECIAL_CTRL;
				}
				if(StringOperations::CompareInsensitive(*str, wstring(L"win")) ||StringOperations::CompareInsensitive(*str, wstring(L"meta")) || 
					StringOperations::CompareInsensitive(*str, wstring(L"super")))
				{
					special |= KEYSPECIAL_WIN;
				}
			}

			return Key<T>(character, special);
		}

		DLLEXPORT wstring GenerateWstringFromKey(){

			// First the actual key value //
			wstring resultstr = Leviathan::Window::ConvertOISKeyCodeToWstring(Character);

			// Add special modifiers //
			if(Extras && KEYSPECIAL_ALT)
				resultstr += L"+ALT";
			if(Extras && KEYSPECIAL_CTRL)
				resultstr += L"+CTRL";
			if(Extras && KEYSPECIAL_SHIFT)
				resultstr += L"+SHIFT";
			if(Extras && KEYSPECIAL_WIN)
				resultstr += L"+META";
			// Result is done //
			return resultstr;
		}

	private:
		short Extras;
		T Character;
	};

	// This is the most likely type //
	typedef Key<OIS::KeyCode> GKey;

}
#endif
