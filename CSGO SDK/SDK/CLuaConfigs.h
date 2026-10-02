#include "sdk.hpp"

#ifndef CONFIGSYSTEM_H_
#define CONFIGSYSTEM_H_

inline std::string ReplaceInString(std::string str, const std::string& oldStr, const std::string& newStr) //stackoverflow is best
{
	std::string finalString;
	std::string::size_type pos = 0u;
	while ((pos = str.find(oldStr, pos)) != std::string::npos) {
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
	finalString = str;

	return finalString;
}

namespace LuaConfigSystem {
	inline std::vector<const char*> presets = { "default" /*too lazy to add multiple presets to menu for now */ };
	inline std::unordered_map<std::string, bool> C_BOOL;
	inline std::unordered_map<std::string, int> C_INT;
	inline std::unordered_map<std::string, float> C_FLOAT;
	inline std::unordered_map<std::string, float[4]> C_COLOR;
	inline 	std::unordered_map<std::string, std::unordered_map<int, bool>> C_MULTI;


	inline Color ImColorToColor(float* col) {
		return Color((int)(col[0] * 255.f), (int)(col[1] * 255.f), (int)(col[2] * 255.f), (int)(col[3] * 255.f));
	}

	inline Color ImColorToColor(float* col, float alpha_override) {
		return Color((int)(col[0] * 255.f), (int)(col[1] * 255.f), (int)(col[2] * 255.f), (int)(alpha_override * 255.f));
	}

	inline std::string ColorToString(float col[4]) {
		return std::to_string((int)(col[0] * 255)) + "," + std::to_string((int)(col[1] * 255)) + "," + std::to_string((int)(col[2] * 255)) + "," + std::to_string((int)(col[3] * 255));
	}

	inline static float ToFloat(int x) {
		return (static_cast<float>(x) / 255.0f);
	}


	inline	float* StringToColor(std::string s) {
		static auto split = [](std::string str, const char* del) -> std::vector<std::string>
		{
			char* pTempStr = _strdup(str.c_str());
			char* pWord = strtok(pTempStr, del);
			std::vector<std::string> dest;

			while (pWord != NULL)
			{
				dest.push_back(pWord);
				pWord = strtok(NULL, del);
			}

			free(pTempStr);

			return dest;
		};

		std::vector<std::string> col = split(s, ",");
		return new float[4]{
			(float)std::stoi(col.at(0)) / 255.f,
			(float)std::stoi(col.at(1)) / 255.f,
			(float)std::stoi(col.at(2)) / 255.f,
			(float)std::stoi(col.at(3)) / 255.f
		};
	}

	inline void Save() {
		char FilePath[MAX_PATH] = { 0 };
		sprintf(FilePath, ReplaceInString(std::filesystem::current_path().string() + "\\vader.tech\\lua.ini", "\\", "/").c_str());

		for (auto e : C_BOOL) {
			if (!std::string(e.first).find("_")) continue;
			char buffer[8] = { 0 }; _itoa(e.second, buffer, 10);
			WritePrivateProfileStringA("b", e.first.c_str(), std::string(buffer).c_str(), FilePath);
		}

		for (auto e : C_INT) {
			if (!std::string(e.first).find("_")) continue;
			char buffer[32] = { 0 }; _itoa(e.second, buffer, 10);
			WritePrivateProfileStringA("i", e.first.c_str(), std::string(buffer).c_str(), FilePath);
		}

		for (auto e : C_FLOAT) {
			if (!std::string(e.first).find("_")) continue;
			char buffer[64] = { 0 }; sprintf(buffer, "%f", e.second);
			WritePrivateProfileStringA("f", e.first.c_str(), std::string(buffer).c_str(), FilePath);
		}

		for (auto e : C_COLOR) {
			if (!std::string(e.first).find("_")) continue;
			WritePrivateProfileStringA("c", e.first.c_str(), ColorToString(e.second).c_str(), FilePath);
		}

		for (auto e : C_MULTI) {
			if (!std::string(e.first).find("_")) continue;

			std::string vs = "";
			for (auto v : e.second)
				vs += std::to_string(v.first) + ":" + std::to_string(v.second) + "|";

			WritePrivateProfileStringA("m", e.first.c_str(), vs.c_str(), FilePath);
		}
	}

	inline void Load() {
		char FilePath[MAX_PATH] = { 0 };
		sprintf(FilePath, ReplaceInString(std::filesystem::current_path().string() + "\\vader.tech\\lua.ini", "\\", "/").c_str());

		char b_buffer[65536], i_buffer[65536], f_buffer[65536], c_buffer[65536], m_buffer[65536] = { 0 };

		int b_read = GetPrivateProfileSectionA("b", b_buffer, 65536, FilePath);
		int i_read = GetPrivateProfileSectionA("i", i_buffer, 65536, FilePath);
		int f_read = GetPrivateProfileSectionA("f", f_buffer, 65536, FilePath);
		int c_read = GetPrivateProfileSectionA("c", c_buffer, 65536, FilePath);
		int m_read = GetPrivateProfileSectionA("m", m_buffer, 65536, FilePath);

		if ((0 < b_read) && ((65536 - 2) > b_read)) {
			const char* pSubstr = b_buffer;

			while ('\0' != *pSubstr) {
				size_t substrLen = strlen(pSubstr);

				const char* pos = strchr(pSubstr, '=');
				if (NULL != pos) {
					char name[256] = "";
					char value[256] = "";

					strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
					strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

					C_BOOL[name] = atoi(value);
				}

				pSubstr += (substrLen + 1);
			}
		}

		if ((0 < i_read) && ((65536 - 2) > i_read)) {
			const char* pSubstr = i_buffer;

			while ('\0' != *pSubstr) {
				size_t substrLen = strlen(pSubstr);

				const char* pos = strchr(pSubstr, '=');
				if (NULL != pos) {
					char name[256] = "";
					char value[256] = "";

					strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
					strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

					C_INT[name] = atoi(value);
				}

				pSubstr += (substrLen + 1);
			}
		}

		if ((0 < f_read) && ((65536 - 2) > f_read)) {
			const char* pSubstr = f_buffer;

			while ('\0' != *pSubstr) {
				size_t substrLen = strlen(pSubstr);

				const char* pos = strchr(pSubstr, '=');
				if (NULL != pos) {
					char name[256] = "";
					char value[256] = "";

					strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
					strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

					C_FLOAT[name] = (float)atof(value);
				}

				pSubstr += (substrLen + 1);
			}
		}

		if ((0 < c_read) && ((65536 - 2) > c_read)) {
			const char* pSubstr = c_buffer;

			while ('\0' != *pSubstr) {
				size_t substrLen = strlen(pSubstr);

				const char* pos = strchr(pSubstr, '=');
				if (NULL != pos) {
					char name[256] = "";
					char value[256] = "";

					strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
					strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

					auto col = StringToColor(value);
					C_COLOR[name][0] = col[0];
					C_COLOR[name][1] = col[1];
					C_COLOR[name][2] = col[2];
					C_COLOR[name][3] = col[3];
				}

				pSubstr += (substrLen + 1);
			}
		}

		static auto split = [](std::string str, const char* del) -> std::vector<std::string>
		{
			char* pTempStr = _strdup(str.c_str());
			char* pWord = strtok(pTempStr, del);
			std::vector<std::string> dest;

			while (pWord != NULL)
			{
				dest.push_back(pWord);
				pWord = strtok(NULL, del);
			}

			free(pTempStr);

			return dest;
		};

		if ((0 < m_read) && ((65536 - 2) > m_read)) {
			const char* pSubstr = m_buffer;

			while ('\0' != *pSubstr) {
				size_t substrLen = strlen(pSubstr);

				const char* pos = strchr(pSubstr, '=');
				if (NULL != pos) {
					char name[256] = "";
					char value[256] = "";

					strncpy_s(name, _countof(name), pSubstr, pos - pSubstr);
					strncpy_s(value, _countof(value), pos + 1, substrLen - (pos - pSubstr));

					std::vector<std::string> kvpa = split(value, "|");
					std::unordered_map<int, bool> vl = {};
					for (auto kvp : kvpa) {
						if (kvp == "")
							continue; // ало глухой

						std::vector<std::string> kv = split(kvp, ":");
						vl[std::stoi(kv.at(0))] = std::stoi(kv.at(1));
					}

					C_MULTI[name] = vl;
				}

				pSubstr += (substrLen + 1);
			}
		}
	}
}

#endif