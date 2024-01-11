#pragma once
#ifndef LIMVARIABLE_H
#define LIMVARIABLE_H

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace lim {
    class LimVar {
    private:
        std::string m_name;
        std::wstring m_value;
        std::wstring m_prefix;
        std::wstring m_sufix;
        bool m_applyPad;
        char m_padChar;
        unsigned int m_padLength;
        std::string m_outputMask;

    public:
        LimVar(std::string_view name = "") :
            m_name{ name },
            m_value{ L"" },
            m_prefix{ L"" },
            m_sufix{ L"" },
            m_outputMask{ "" },
            m_applyPad{ false },
            m_padChar{ '0' },
            m_padLength{ 0 }
        {};

        const std::string& name() const { return m_name; }

        void setValue(std::wstring_view value) { m_value = value; }
        const std::wstring& value() const { return m_value; }

        void setPretix(std::wstring_view prefix) { m_prefix = prefix; }
        const std::wstring& prefix() const { return m_prefix; }

        void setSufix(std::wstring_view sufix) { m_sufix = sufix; }
        const std::wstring& sufix() const { return m_sufix; }

        void setOutputMask(std::string_view outputMask) { m_outputMask = outputMask; }
        const std::string& outputMask() const { return m_outputMask; }

        void setApplyPad(bool applyPad) { m_applyPad = applyPad; }
        bool applyPad() const { return m_applyPad; }

        void setPadChar(char padChar) { m_padChar = padChar; }
        char padChar() const { return m_padChar; }

        void setPadLength(unsigned int padLength) { m_padLength = padLength; }
        unsigned int padLength() const { return m_padLength; }

        const std::wstring getVariableValue() const;

        virtual const std::wstring calcValue() const = 0;
        virtual const json to_json() const = 0;

        virtual void from_json(const json& j);
    };

    class LimStringVar : public LimVar {
    private:
        std::wstring m_textValue;

    public:
        LimStringVar(std::string_view name = "") :
            LimVar(name),
            m_textValue{ L"" }
        {};

        const std::wstring calcValue() const override
        { 
            return m_textValue;
        }

        const json to_json() const override;
        void from_json(const json& j) override;

        void setTextValue(std::wstring_view textValue) { m_textValue = textValue; }
        const std::wstring& textValue() const { return m_textValue; }
    };

    enum PeriodType { perSecond, perMinute, perHour, perDay, perMonth };

    class LimDTVariable : public LimVar {
    private:
        bool m_fixed;
        //std::chrono::time_point m_fixedValue{ };
        int m_addPeriod;
        PeriodType m_periodType;
        std::string m_format;
        std::string m_language;
    };

    enum CounterType { cntDec, cntHex, cntBin };
    class LimCounterVariable : public LimVar {
    private:
        CounterType m_type;
        unsigned long m_currentValue;
        unsigned int m_increment;
        unsigned long m_minValue;
        unsigned long m_maxValue;
        bool m_overflow;
        unsigned int m_printsToIncrement;
    };
}

#endif