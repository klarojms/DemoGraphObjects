#include "LimVariable.h"
#include "ToolsJosep.h"

// ----------------------------------------------------------------------------------------
// LimVar
// ----------------------------------------------------------------------------------------

void lim::LimVar::from_json(const json& j)
{
	if (j.contains("name"))
		j.at("name").get_to(m_name);
	if (j.contains("prefix")) {
		std::string str;
		j.at("prefix").get_to(str);
		m_prefix = utf8_decode(str);
	}
	if (j.contains("sufix")) {
		std::string str;
		j.at("sufix").get_to(str);
		m_sufix = utf8_decode(str);
	}
	if (j.contains("applyPad"))
		j.at("applyPad").get_to(m_applyPad);
	if (j.contains("padChar")) {
		std::string str;
		j.at("padChar").get_to(str);
		if(str.size())
			m_padChar = str[0];
	}
	if (j.contains("padLength"))
		j.at("padLength").get_to(m_padLength);
	if (j.contains("outputMask"))
		j.at("outputMask").get_to(m_outputMask);
}

const std::wstring lim::LimVar::getVariableValue() const
{
	std::wstring result;

	result = m_prefix;

	if (m_applyPad) {
		if (m_value.size() < m_padLength)
			result += std::wstring(m_padLength - m_value.size(), m_padChar);
	}

	result += m_value + m_sufix;

	return result;
}

// ----------------------------------------------------------------------------------------
// LimStringVar
// ----------------------------------------------------------------------------------------

const json lim::LimStringVar::to_json() const
{
	const std::wstring ws1 = prefix();
	std::string prefix = utf8_encode(ws1);
	const std::wstring ws2 = sufix();
	std::string sufix = utf8_encode(ws2);
	std::string textValue = utf8_encode(m_textValue);
	std::string padCh;  
	padCh.push_back(padChar());

	json j = {
		{"type", "text"},
		{"name", name()},
		{"prefix", prefix},
		{"sufix", sufix},
		{"applyPad", applyPad()},
		{"padChar", padCh},
		{"padLength", padLength()},
		{"outputMask", outputMask()},
		{"textValue", textValue}
	};

	return j;
}

void lim::LimStringVar::from_json(const json& j)
{
	LimVar::from_json(j);
	if (j.contains("textValue")) {
		std::string str;
		j.at("textValue").get_to(str);
		m_textValue = utf8_decode(str);
	}
}
