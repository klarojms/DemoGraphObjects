#pragma once
#ifndef LIMLAYOUT_H
#define LIMLAYOUT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <tbarcode.h>
#include <tecbcenum.h>
#include <nlohmann/json.hpp>
#include "LimVariable.h"

using json = nlohmann::json;

namespace lim {
    class ObjGraph {
    private:
        cv::Rect m_rect;

    public:
        ObjGraph() : m_rect{ 0, 0, 0, 0 } {};
        ObjGraph(const cv::Rect& rect) : m_rect{ rect } {};

        virtual ~ObjGraph() {}; // Para poder ejecutar destructores override

        cv::Rect& getRect() { return m_rect; }
        cv::Rect getRect() const { return m_rect; }

        virtual void draw(cv::Mat&) = 0;
        virtual const json toJSON() const = 0;
        virtual void from_json(const json& j);

        static void rotate(cv::Mat& imgSrc, cv::Mat& imgDst, cv::Rect& realRect, int angle);
    };

    class Shape : public ObjGraph
    {
        int m_thickness;
    public:
        Shape(int thickness = 1) : ObjGraph(), m_thickness{ thickness } {};
        Shape(cv::Rect rect, int thickness = 1) : ObjGraph{ rect }, m_thickness{ thickness } {};

        int getThickness() const { return m_thickness; }
        void setThickness(int thickness) { m_thickness = thickness; }

        void from_json(const json& j) override;
    };

    class Line : public Shape
    {
    public:
        Line(int thickness = 1) : Shape(thickness) {};
        Line(cv::Rect rect, int thickness = 1) : Shape{ rect, thickness } {};

        void draw(cv::Mat& img) override
        {
            cv::line(img, getRect().tl(), getRect().br(), cv::Scalar::all(0), getThickness());
        }
       
        const json toJSON() const override;
    };

    class Rectangle : public Shape
    {
    public:
        Rectangle(int thickness = 1) : Shape(thickness) {};
        Rectangle(cv::Rect rect, int thickness = 1) : Shape{ rect, thickness } {};

        void draw(cv::Mat& img) override
        {
            cv::rectangle(img, getRect(), cv::Scalar::all(0), getThickness());
        }

        const json toJSON() const override;
    };

    class Ellipse : public Shape
    {
    public:
        Ellipse(int thickness = 1) : Shape(thickness) {};
        Ellipse(cv::Rect rect, int thickness = 1) : Shape{ rect, thickness } {};

        void draw(cv::Mat& img) override
        {
            cv::Rect rect = getRect();
            cv::Point center{ rect.tl().x + rect.width / 2, rect.tl().y + rect.height / 2 };
            cv::Size axes{ rect.width / 2, rect.height / 2 };
            cv::ellipse(img, center, axes, 0, 0, 360, cv::Scalar::all(0), getThickness());
        }

        const json toJSON() const override;
    };

    class Text : public ObjGraph
    {
    private:
        std::wstring m_text;
        std::string m_font;
        int m_fontHeight;
        int m_angle;
        bool m_drawRect;
        std::string m_variable;

    public:
        Text(std::wstring_view text = L"", std::string_view font = "", int fontHeight = 10, int angle = 0, bool drawRect=false) :
            ObjGraph{},
            m_text{ text },
            m_font{ font },
            m_fontHeight{ fontHeight },
            m_angle{ angle },
            m_drawRect{drawRect},
            m_variable{""}
        {
        };

        Text(cv::Rect rect, std::wstring_view text = L"", std::string_view font = "", int fontHeight = 10, int angle = 0, bool drawRect = false) :
            ObjGraph{ rect },
            m_text{ text },
            m_font{ font },
            m_fontHeight{ fontHeight },
            m_angle{ angle },
            m_drawRect{ drawRect },
            m_variable{ "" }
        {
        };

        void draw(cv::Mat& img) override;

        const std::wstring& text() const { return m_text; }
        void setText(std::wstring_view text) { m_text = text; }

        const std::string& variable() const { return m_variable; }

        const json toJSON() const override;
        void from_json(const json& j) override;
    };

    class Barcode : public ObjGraph
    {
    private:
        t_BarCode* m_barCodeHandle;         // barcode handle

        ERRCODE m_errorCode;
        e_BarCType m_typeBarcode;
        std::wstring m_code;
        bool m_showLabel;
        bool m_textAbove;
        bool m_rectangleGuard;
        unsigned int m_fontSize;
        std::string m_fontName;
        byte m_bwr;
        e_QRECLevel m_QRECLevel;
        e_QRFormat m_QRFormat;
        e_DMFormat m_DMFormat;
        int m_bearerBarWidth;
        bool m_autoCheckDigit;
        unsigned int m_angle;
        unsigned int m_segmentsByRow;
        e_SizeMode m_sizeMode;
        unsigned int m_sizeModules;
        unsigned int m_modulesQZ;
        std::string m_variable;

    public:
        Barcode() :
            ObjGraph{},
            m_errorCode{ 0 },
            m_barCodeHandle{ nullptr },
            m_typeBarcode{ eBC_EAN13 },
            m_code{ L"8432214000002" },
            m_showLabel{ true },
            m_textAbove{ false },
            m_rectangleGuard{ false },
            m_fontName{ "Arial" },
            m_fontSize{ 10 },
            m_bwr{ 0 },
            m_QRECLevel{ eQREC_Medium },
            m_QRFormat{ eQRPr_Default },
            m_DMFormat{ eDMPr_Default },
            m_bearerBarWidth{ 1 },
            m_autoCheckDigit{ true },
            m_angle{ 0 },
            m_segmentsByRow{ 6 },
            m_sizeMode{ eSizeMode_FitToBoundingRect },
            m_sizeModules{ 5 },
            m_modulesQZ{ 0 },
            m_variable{ "" }
        {
            m_errorCode = BCAlloc(&m_barCodeHandle);
        }

        Barcode(cv::Rect rect) :
            ObjGraph{ rect },
            m_errorCode{ 0 },
            m_barCodeHandle{ nullptr },
            m_typeBarcode{ eBC_EAN13 },
            m_code{ L"8432214000002" },
            m_showLabel{ true },
            m_textAbove{ false },
            m_rectangleGuard{ false },
            m_fontName{ "Arial" },
            m_fontSize{ 10 },
            m_bwr{ 0 },
            m_QRECLevel{ eQREC_Medium },
            m_QRFormat{ eQRPr_Default },
            m_DMFormat{ eDMPr_Default },
            m_bearerBarWidth{ 1 },
            m_autoCheckDigit{ true },
            m_angle{ 0 },
            m_segmentsByRow{ 6 },
            m_sizeMode{ eSizeMode_FitToBoundingRect },
            m_sizeModules{ 5 },
            m_modulesQZ{ 0 },
            m_variable{ "" }
        {
            m_errorCode = BCAlloc(&m_barCodeHandle);
        }

        ~Barcode() override
        {
            // Recordar definir el destructor padre como virtual
            m_errorCode = BCFree(m_barCodeHandle);
        }

        void setType(e_BarCType typeBarcode) { m_typeBarcode = typeBarcode; }

        e_BarCType typeBarcode() const { return m_typeBarcode; }

        void setCode(std::wstring_view code) { m_code = code; }

        const std::wstring& code() const { return m_code; }

        ERRCODE errorCode() const { return m_errorCode; }

        void setShowLabel(bool showLabel) { m_showLabel = showLabel; }

        bool showLabels() const { return m_showLabel; }

        void setTextAbove(bool textAbove) { m_textAbove = textAbove; }

        bool textAbove() const { return m_textAbove; }

        void setRectangleGuard(bool rectangleGuard) { m_rectangleGuard = rectangleGuard; }

        bool rectangleGuard() const { return m_rectangleGuard; }

        void setFontName(std::string_view code) { m_fontName = code; }

        const std::string& fontName() const { return m_fontName; }
        
        void setFontSize(unsigned int fontSize) { m_fontSize = fontSize; }

        unsigned int fontSize() const { return m_fontSize; }

        void setBwr(byte bwr) { m_bwr = bwr; }

        byte bwr() const { return m_bwr; }

        void setQRECLevel(e_QRECLevel QRECLevel) { m_QRECLevel = QRECLevel; }

        e_QRECLevel QRECLevel() const { return m_QRECLevel; }

        void setQRFormat(e_QRFormat QRFormat) { m_QRFormat = QRFormat; }

        e_QRFormat QRFormat() const { return m_QRFormat; }

        void setDMFormat(e_DMFormat DMFormat) { m_DMFormat = DMFormat; }

        e_DMFormat DMFormat() const { return m_DMFormat; }

        void setBearerBarWidth(int bearerBarWidth) { m_bearerBarWidth = bearerBarWidth; }

        int bearerBarWidth() const { return m_bearerBarWidth; }

        void setSegmentsByRow(unsigned int segmentsByRow) { m_segmentsByRow = segmentsByRow; }

        unsigned int segmentsByRow() const { return m_segmentsByRow; }

        void setAngle(unsigned int angle) { m_angle = angle; }

        unsigned int getAngle() const { return m_angle; }

        void setAutoCheckDigit(bool autoCheckDigit) { m_autoCheckDigit = autoCheckDigit; }

        bool autoCheckDigit() const { return m_autoCheckDigit; }

        void setSizeMode(e_SizeMode sizeMode) { m_sizeMode = sizeMode; }

        e_SizeMode sizeMode() const { return m_sizeMode; }

        void setSizeModules(unsigned int sizeModules) { m_sizeModules = sizeModules; }

        unsigned int sizeModules() const { return m_sizeModules; }

        void setModulesQZ(unsigned int modulesQZ) { m_modulesQZ = modulesQZ; }

        unsigned int modulesQZ() const { return m_modulesQZ; }

        const std::string& variable() const { return m_variable; }

        bool prepareBarcode();
            
        void draw(cv::Mat& img) override;

        const json toJSON() const override;

        void from_json(const json& j) override;
    };

    class Image : public ObjGraph
    {
    private:
        std::string m_path;
        bool m_embeded;
        cv::Mat m_img;
        unsigned int m_angle;
    public:
        Image() :
            ObjGraph{},
            m_path{},
            m_embeded{ false },
            m_angle{ 0 }
        {
        }

        Image(cv::Rect rect) :
            ObjGraph{ rect },
            m_path{},
            m_embeded{ false },
            m_angle{ 0 }
        {
        }

        Image(cv::Rect rect, std::string_view path, bool embeded=false) : 
            ObjGraph{rect},
            m_path{ path },
            m_embeded{ embeded },
            m_angle{ 0 }
        {
        };

        int angle() const { return m_angle; }
        int setAngle(unsigned int angle) { m_angle = angle; }

        void draw(cv::Mat& img) override;

        const json toJSON() const override;

        void from_json(const json& j) override;
    };

    class LimLayout
    {
    private:
        int m_width{};
        int m_height{};

        std::vector<std::unique_ptr<ObjGraph>> m_objects;
        std::map<std::string, std::unique_ptr<LimVar>> m_variables;

        cv::Mat* img;

    public:
        LimLayout(int width = 0, int height = 0) :
            m_width{ width },
            m_height{ height },
            m_objects{},
            m_variables{},
            img{ nullptr }
        {
        };

        ~LimLayout()
        {
        }

        std::vector<std::unique_ptr<ObjGraph>>& objects() { return m_objects; }

        std::map<std::string, std::unique_ptr<LimVar>>& variables() { return m_variables; }

        void addVariable(std::unique_ptr<LimVar>&& variable);

        void calcVariables();

        void draw();

        cv::Mat* getImage() { return img; }

        std::wstring getVariableValue(std::string_view variable);

        void writeToFile(const std::string& fileName)
        {
            std::ofstream f(fileName);
            if (f.is_open()) {
                f << *this;
                f.close();
            }
        }

        void readFromFile(const std::string& fileName);

        friend std::ostream& operator<<(std::ostream& out, const LimLayout& layout)
        {
            json j;
            j["width"] = layout.m_width;
            j["height"] = layout.m_height;
            j["variables"] = {};
            for (auto itr = layout.m_variables.begin(); itr != layout.m_variables.end(); ++itr)
            {
                lim::LimVar* obj = itr->second.get();
                if (obj)
                    j["variables"].push_back(obj->to_json());
            }
            j["objects"] = {};
            for (auto i = 0; i < layout.m_objects.size(); ++i)
            {
                lim::ObjGraph* obj = layout.m_objects[i].get();
                if (obj)
                    j["objects"].push_back(obj->toJSON());
            }

            out << j.dump(2);

            return out;
        }

    };
}

#endif