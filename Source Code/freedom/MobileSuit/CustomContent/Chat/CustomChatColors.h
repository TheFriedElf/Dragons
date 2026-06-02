#pragma once

#include <map>
#include <string>

#include "Lohengrin/packetstruct.h"

class CustomChatColors
{
public:
    static CustomChatColors& GetInstance();
    static bool NormalizeChatColorType(EChatType& eChatType);
    static bool UseReceivedChatColor(EChatType eChatType);
    static void EnsureOptionDefaults(std::map<std::string, std::pair<int, std::string> >& rkConfigMap);

    bool Load();
    bool Save() const;

    void SetColor(EChatType eChatType, std::wstring const& color);
    std::wstring GetColor(EChatType eChatType) const;
    unsigned int ResolveCustomChatColor(EChatType eChatType, unsigned int dwDefaultColor) const;
    unsigned int ResolveDisplayedChatColor(EChatType eChatType, unsigned int dwReceivedColor, unsigned int dwDefaultColor) const;
    void SaveCustomChatColor(EChatType eChatType, unsigned int dwColor);

private:
    CustomChatColors();

    void ResetDefaults();
    std::map<EChatType, std::wstring> m_colors;
};