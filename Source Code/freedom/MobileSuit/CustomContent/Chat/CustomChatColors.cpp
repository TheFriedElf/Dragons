#include "StdAfx.h"
#include "CustomChatColors.h"
#include "PgOption.h"

namespace
{
	char const* const STR_OPTION_CHAT_COLORS = "CHATCOLORS";
	char const* const STR_OPTION_CHAT_COLOR_REGULAR = "REGULAR";
	char const* const STR_OPTION_CHAT_COLOR_PARTY = "PARTY";
	char const* const STR_OPTION_CHAT_COLOR_FRIEND = "FRIEND";
	char const* const STR_OPTION_CHAT_COLOR_TRADE = "TRADE";
	char const* const STR_OPTION_CHAT_COLOR_GUILD = "GUILD";
	char const* const STR_OPTION_CHAT_COLOR_WHISPER = "WHISPER";

	struct SChatColorDefault
	{
		char const* m_szKey;
		char const* m_szColor;
	};

	SChatColorDefault const g_akChatColorDefaults[] =
	{
		{ STR_OPTION_CHAT_COLOR_REGULAR, "FFFFFF" },
		{ STR_OPTION_CHAT_COLOR_PARTY, "46F8FF" },
		{ STR_OPTION_CHAT_COLOR_FRIEND, "88FF47" },
		{ STR_OPTION_CHAT_COLOR_TRADE, "FFF440" },
		{ STR_OPTION_CHAT_COLOR_GUILD, "FD90FE" },
		{ STR_OPTION_CHAT_COLOR_WHISPER, "FF8249" },
	};

	std::wstring Trimmed(std::wstring const& value)
	{
		std::wstring::size_type const firstNonWhitespacePosition = value.find_first_not_of(L" \t\r\n");
		if (firstNonWhitespacePosition == std::wstring::npos)
		{
			return L"";
		}

		std::wstring::size_type const lastNonWhitespacePosition = value.find_last_not_of(L" \t\r\n");
		return value.substr(firstNonWhitespacePosition, lastNonWhitespacePosition - firstNonWhitespacePosition + 1);
	}

	bool IsValidHexColor(std::wstring const& color)
	{
		if (color.length() != 6)
		{
			return false;
		}

		for (std::wstring::const_iterator characterIterator = color.begin(); characterIterator != color.end(); ++characterIterator)
		{
			if ((*characterIterator < L'0' || *characterIterator > L'9')
				&& (*characterIterator < L'A' || *characterIterator > L'F')
				&& (*characterIterator < L'a' || *characterIterator > L'f'))
			{
				return false;
			}
		}

		return true;
	}

	std::wstring NormalizeHexColor(std::wstring color)
	{
		color = Trimmed(color);

		if (!color.empty() && color[0] == L'#')
		{
			color.erase(0, 1);
		}

		if (color.length() > 2 && color[0] == L'0' && (color[1] == L'x' || color[1] == L'X'))
		{
			color.erase(0, 2);
		}

		if (color.length() == 8)
		{
			color = color.substr(2);
		}

		if (!IsValidHexColor(color))
		{
			return L"";
		}

		for (std::wstring::iterator characterIterator = color.begin(); characterIterator != color.end(); ++characterIterator)
		{
			if (*characterIterator >= L'a' && *characterIterator <= L'f')
			{
				*characterIterator = static_cast<wchar_t>(*characterIterator - (L'a' - L'A'));
			}
		}

		return color;
	}

	char const* GetConfigKey(EChatType eChatType)
	{
		switch (eChatType)
		{
		case CT_NORMAL:
			return STR_OPTION_CHAT_COLOR_REGULAR;
		case CT_PARTY:
			return STR_OPTION_CHAT_COLOR_PARTY;
		case CT_FRIEND:
			return STR_OPTION_CHAT_COLOR_FRIEND;
		case CT_TRADE:
			return STR_OPTION_CHAT_COLOR_TRADE;
		case CT_GUILD:
			return STR_OPTION_CHAT_COLOR_GUILD;
		case CT_WHISPER_BYGUID:
			return STR_OPTION_CHAT_COLOR_WHISPER;
		default:
			return NULL;
		}
	}

	std::wstring WidenAscii(char const* value)
	{
		std::wstring wideValue;
		if (value == NULL)
		{
			return wideValue;
		}

		while (*value != '\0')
		{
			wideValue.push_back(static_cast<wchar_t>(*value));
			++value;
		}

		return wideValue;
	}

	std::string NarrowAscii(std::wstring const& value)
	{
		std::string narrowValue;
		narrowValue.reserve(value.length());

		for (std::wstring::const_iterator valueIterator = value.begin(); valueIterator != value.end(); ++valueIterator)
		{
			narrowValue.push_back(static_cast<char>(*valueIterator));
		}

		return narrowValue;
	}
}

CustomChatColors& CustomChatColors::GetInstance()
{
	static CustomChatColors instance;
	static bool const loaded = instance.Load();
	(void)loaded;
	return instance;
}

bool CustomChatColors::NormalizeChatColorType(EChatType& eChatType)
{
	switch (eChatType)
	{
	case CT_NORMAL:
	case CT_PARTY:
	case CT_FRIEND:
	case CT_TRADE:
	case CT_GUILD:
		return true;
	case CT_WHISPER_BYNAME:
		eChatType = CT_WHISPER_BYGUID;
		return true;
	case CT_WHISPER_BYGUID:
		return true;
	default:
		return false;
	}
}

bool CustomChatColors::UseReceivedChatColor(EChatType eChatType)
{
	switch (eChatType)
	{
	case CT_NORMAL:
	case CT_PARTY:
	case CT_TEAM:
	case CT_WHISPER_BYGUID:
	case CT_WHISPER_BYNAME:
	case CT_TRADE:
	case CT_GUILD:
		return true;
	default:
		return false;
	}
}

CustomChatColors::CustomChatColors()
{
	ResetDefaults();
}

void CustomChatColors::EnsureOptionDefaults(std::map<std::string, std::pair<int, std::string> >& rkConfigMap)
{
	for (size_t i = 0; i < _countof(g_akChatColorDefaults); ++i)
	{
		std::string kFullKey = STR_OPTION_CHAT_COLORS;
		kFullKey.append(STR_OPTION_SEPARATER);
		kFullKey.append(g_akChatColorDefaults[i].m_szKey);

		if (rkConfigMap.find(kFullKey) == rkConfigMap.end())
		{
			rkConfigMap.insert(std::make_pair(kFullKey, std::make_pair(0, std::string(g_akChatColorDefaults[i].m_szColor))));
		}
	}
}

void CustomChatColors::ResetDefaults()
{
	m_colors[CT_NORMAL] = L"FFFFFF";
	m_colors[CT_PARTY] = L"46F8FF";
	m_colors[CT_FRIEND] = L"88FF47";
	m_colors[CT_TRADE] = L"FFF440";
	m_colors[CT_GUILD] = L"FD90FE";
	m_colors[CT_WHISPER_BYGUID] = L"FF8249";
}

bool CustomChatColors::Load()
{
	ResetDefaults();
	bool bLoaded = false;

	EChatType aeChatTypes[] =
	{
		CT_NORMAL,
		CT_PARTY,
		CT_FRIEND,
		CT_TRADE,
		CT_GUILD,
		CT_WHISPER_BYGUID,
	};

	for (size_t i = 0; i < _countof(aeChatTypes); ++i)
	{
		EChatType eChatType = aeChatTypes[i];
		char const* szKey = GetConfigKey(eChatType);
		if (szKey == NULL)
		{
			continue;
		}

		char const* szColorText = g_kGlobalOption.GetText(STR_OPTION_CHAT_COLORS, szKey);
		if (szColorText == NULL || szColorText[0] == '\0')
		{
			szColorText = g_kGlobalOption.GetDefaultText(STR_OPTION_CHAT_COLORS, szKey);
		}

		std::wstring const colorValue = NormalizeHexColor(WidenAscii(szColorText));
		if (colorValue.empty())
		{
			continue;
		}

		m_colors[eChatType] = colorValue;
		bLoaded = true;
	}

	return bLoaded;
}

bool CustomChatColors::Save() const
{
	EChatType aeChatTypes[] =
	{
		CT_NORMAL,
		CT_PARTY,
		CT_FRIEND,
		CT_TRADE,
		CT_GUILD,
		CT_WHISPER_BYGUID,
	};

	for (size_t i = 0; i < _countof(aeChatTypes); ++i)
	{
		EChatType eChatType = aeChatTypes[i];
		char const* szKey = GetConfigKey(eChatType);
		if (szKey == NULL)
		{
			continue;
		}

		std::string const colorValue = NarrowAscii(GetColor(eChatType));
		if (!g_kGlobalOption.SysSetConfig(STR_OPTION_CHAT_COLORS, szKey, 0, colorValue.c_str()))
		{
			return false;
		}
	}

	return g_kGlobalOption.Save();
}

void CustomChatColors::SetColor(EChatType eChatType, std::wstring const& color)
{
	if (!NormalizeChatColorType(eChatType))
	{
		return;
	}

	std::wstring const normalizedColor = NormalizeHexColor(color);
	if (!normalizedColor.empty())
	{
		m_colors[eChatType] = normalizedColor;
	}
}

std::wstring CustomChatColors::GetColor(EChatType eChatType) const
{
	if (!NormalizeChatColorType(eChatType))
	{
		return L"FFFFFF";
	}

	std::map<EChatType, std::wstring>::const_iterator colorIterator = m_colors.find(eChatType);
	if (colorIterator != m_colors.end())
	{
		return colorIterator->second;
	}

	return L"FFFFFF";
}

unsigned int CustomChatColors::ResolveCustomChatColor(EChatType eChatType, unsigned int dwDefaultColor) const
{
	if (!NormalizeChatColorType(eChatType))
	{
		return dwDefaultColor;
	}

	std::wstring const colorText = GetColor(eChatType);
	if (colorText.empty())
	{
		return dwDefaultColor;
	}

	wchar_t* parseEnd = NULL;
	unsigned long const parsedColorValue = wcstoul(colorText.c_str(), &parseEnd, 16);
	if (parseEnd == colorText.c_str() || (parseEnd != NULL && *parseEnd != L'\0'))
	{
		return dwDefaultColor;
	}

	return 0xFF000000 | (static_cast<unsigned int>(parsedColorValue) & 0x00FFFFFF);
}

unsigned int CustomChatColors::ResolveDisplayedChatColor(EChatType eChatType, unsigned int dwReceivedColor, unsigned int dwDefaultColor) const
{
	if (UseReceivedChatColor(eChatType) && dwReceivedColor != 0)
	{
		return dwReceivedColor;
	}

	return ResolveCustomChatColor(eChatType, dwDefaultColor);
}

void CustomChatColors::SaveCustomChatColor(EChatType eChatType, unsigned int dwColor)
{
	if (!NormalizeChatColorType(eChatType))
	{
		return;
	}

	wchar_t colorBuffer[16] = { 0, };
	swprintf_s(colorBuffer, _countof(colorBuffer), L"%06X", dwColor & 0x00FFFFFF);

	SetColor(eChatType, colorBuffer);
	Save();
}