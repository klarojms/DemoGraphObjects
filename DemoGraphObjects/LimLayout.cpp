#include "LimLayout.h"
//#include <opencv2/freetype.hpp>
#include <regex>
#include <Windows.h>
#include <string>
#include "base64.h"
#include "ToolsJosep.h"

// ----------------------------------------------------------------------------------------
// ObjGraph
// ----------------------------------------------------------------------------------------

void lim::ObjGraph::from_json(const json& j)
{
    if (j.contains("x"))
        j.at("x").get_to(m_rect.x);
    if (j.contains("y"))
        j.at("y").get_to(m_rect.y);
    if (j.contains("width"))
        j.at("width").get_to(m_rect.width);
    if (j.contains("height"))
        j.at("height").get_to(m_rect.height);
}

void lim::ObjGraph::rotate(cv::Mat& imgSrc, cv::Mat& imgDst, cv::Rect& realRect, int angle)
{
    if (angle == 90 || angle == 270)
    {
        cv::rotate(imgSrc, imgSrc, angle == 90 ? cv::ROTATE_90_CLOCKWISE : cv::ROTATE_90_COUNTERCLOCKWISE);

        cv::Point center{ realRect.x + realRect.width / 2, realRect.y + realRect.height / 2 };
        cv::Rect rotRect{
            center.x - realRect.height / 2,
            center.y - realRect.width / 2,
            realRect.height,
            realRect.width
        };

        if ((rotRect.y + rotRect.height) > imgDst.rows)
            rotRect.height = imgDst.rows - rotRect.y;
        cv::Mat insertImage(imgDst, rotRect);
        imgSrc.copyTo(insertImage);
    }
    else
    {
        // Para rotación de 0 y 180
        cv::Mat insertImage(imgDst, realRect);
        if (angle == 180)
            cv::rotate(imgSrc, insertImage, cv::ROTATE_180);
        else
            imgSrc.copyTo(insertImage);
    }
}

// ----------------------------------------------------------------------------------------
// Shape
// ----------------------------------------------------------------------------------------

void lim::Shape::from_json(const json& j) 
{
    ObjGraph::from_json(j);
    if (j.contains("thickness"))
        j.at("thickness").get_to(m_thickness);
}

// ----------------------------------------------------------------------------------------
// Line
// ----------------------------------------------------------------------------------------

const json lim::Line::toJSON() const
{
    const cv::Rect& r = getRect();

    json j = {
        {"type", "line"},
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height},
        {"thickness", getThickness()}
    };

    return j;
}

// ----------------------------------------------------------------------------------------
// Rectangle
// ----------------------------------------------------------------------------------------

const json lim::Rectangle::toJSON() const
{
    const cv::Rect& r = getRect();

    json j = {
        {"type", "rectangle"},
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height},
        {"thickness", getThickness()}
    };

    return j;
}

// ----------------------------------------------------------------------------------------
// Ellipse
// ----------------------------------------------------------------------------------------

const json lim::Ellipse::toJSON() const
{
    const cv::Rect& r = getRect();

    json j = {
        {"type", "ellipse"},
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height},
        {"thickness", getThickness()}
    };

    return j;
}

// ----------------------------------------------------------------------------------------
// Text
// ----------------------------------------------------------------------------------------

const json lim::Text::toJSON() const
{
    const cv::Rect& r = getRect();

    std::string encoded = utf8_encode(m_text);
    json j = {
        {"type", "text"},
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height},
        {"text", encoded},
        {"font", m_font},
        {"fontHeight", m_fontHeight},
        {"angle", m_angle},
        {"drawRect", m_drawRect},
        {"variable", m_variable}
    };

    return j;
}

void lim::Text::from_json(const json& j)
{
    ObjGraph::from_json(j);
    if (j.contains("text")) {
        std::string str;
        j.at("text").get_to(str);
        m_text = utf8_decode(str);
    }
    if (j.contains("font"))
        j.at("font").get_to(m_font);
    if (j.contains("fontHeight"))
        j.at("fontHeight").get_to(m_fontHeight);
    if (j.contains("angle"))
        j.at("angle").get_to(m_angle);
    if (j.contains("drawRect"))
        j.at("drawRect").get_to(m_drawRect);
    if (j.contains("variable")) {
        j.at("variable").get_to(m_variable);
    }
}

void lim::Text::draw(cv::Mat& img)
{
    cv::Rect& rect = getRect();

    HBITMAP hbmp = CreateBitmap(rect.width, rect.height, 1, 0, 0);
    HDC hdcimage = CreateCompatibleDC(NULL);
    SelectObject(hdcimage, hbmp);

    SetMapMode(hdcimage, MM_TEXT);
    //HDC hdc = BeginPaint(hdcimage, &ps);
    long lfHeight = m_fontHeight;

    std::wstring fontName = std::wstring(m_font.begin(), m_font.end());
    HFONT hFont = CreateFont(lfHeight, 
        0,	// Font Width (0 = auto)
        0,	// Escapement
        0,	// Orientation
        FW_NORMAL,	// Weight
        FALSE,	// Italic
        FALSE,	// Underline
        FALSE, // Strike-Out
        DEFAULT_CHARSET, // Character Set
        OUT_DEFAULT_PRECIS, // Output Precision
        CLIP_DEFAULT_PRECIS, // Clip Precision
        DEFAULT_QUALITY, // Quality
        DEFAULT_PITCH | FF_DONTCARE, 
        fontName.c_str());
    
    SelectObject(hdcimage, hFont);

    SetTextColor(hdcimage, RGB(0, 0, 0));

    //std::wstring textOut = std::wstring(m_text.begin(), m_text.end());

    SetBkMode(hdcimage, OPAQUE);

    RECT rOut{ 0, 0, rect.width, rect.height };
    
    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdcimage, &rOut, brush);
    DeleteObject(brush);

    //std::wstring l{ L"Limitronic\n012345\nèáñôç" };
    DrawText(hdcimage, m_text.c_str(), -1, &rOut, DT_LEFT | DT_TOP);
    DeleteObject(hFont);

    if(m_drawRect) 
    {
        brush = CreateSolidBrush(RGB(0, 0, 0));
        FrameRect(hdcimage, &rOut, brush);
        DeleteObject(brush);
    }

    //rect.width = size.cx;

    cv::Mat imgText = cv::Mat(rect.height, rect.width, CV_8UC4, cv::Scalar::all(255));
    BITMAPINFOHEADER bi = { sizeof(bi), rect.width, -rect.height, 1, 32, BI_RGB };
    GetDIBits(hdcimage, hbmp, 0, rect.height, imgText.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    ObjGraph::rotate(imgText, img, rect, m_angle);

    DeleteDC(hdcimage);
    DeleteObject(hbmp);
}

// ----------------------------------------------------------------------------------------
// Barcode
// ----------------------------------------------------------------------------------------

const json lim::Barcode::toJSON() const
{
    const cv::Rect& r = getRect();

    std::string encoded = utf8_encode(m_code);

    json j = {
        {"type", "barcode"},
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height},
        {"typeBarcode", m_typeBarcode},
        {"code", encoded},
        {"showLabel", m_showLabel},
        {"textAbove", m_textAbove},
        {"rectangleGuard", m_rectangleGuard},
        {"fontName", m_fontName},
        {"fontSize", m_fontSize},
        {"bwr", m_bwr},
        {"QRECLevel", m_QRECLevel},
        {"QRFormat", m_QRFormat},
        {"DMFormat", m_DMFormat},
        {"bearerBarWidth", m_bearerBarWidth},
        {"autoCheckDigit", m_autoCheckDigit},
        {"angle", m_angle},
        {"segmentsByRow", m_segmentsByRow},
        {"sizeMode", m_sizeMode},
        {"sizeModules", m_sizeModules},
        {"modulesQZ", m_modulesQZ},
        {"variable", m_variable}
    };

    return j;
}

void lim::Barcode::from_json(const json& j)
{
    //Barcode& b = dynamic_cast<Barcode&>(o);
    ObjGraph::from_json(j);
    if (j.contains("typeBarcode"))
        j.at("typeBarcode").get_to(m_typeBarcode);
    if (j.contains("code")) {
        std::string str;
        j.at("code").get_to(str);
        m_code = utf8_decode(str);
    }
    if (j.contains("showLabel"))
        j.at("showLabel").get_to(m_showLabel);
    if (j.contains("textAbove"))
        j.at("textAbove").get_to(m_textAbove);
    if (j.contains("rectangleGuard"))
        j.at("rectangleGuard").get_to(m_rectangleGuard);
    if (j.contains("fontName"))
        j.at("fontName").get_to(m_fontName);
    if (j.contains("fontSize"))
        j.at("fontSize").get_to(m_fontSize);
    if (j.contains("bwr"))
        j.at("bwr").get_to(m_bwr);
    if (j.contains("QRECLevel"))
        j.at("QRECLevel").get_to(m_QRECLevel);
    if (j.contains("QRFormat"))
        j.at("QRFormat").get_to(m_QRFormat);
    if (j.contains("DMFormat"))
        j.at("DMFormat").get_to(m_DMFormat);
    if (j.contains("bearerBarWidth"))
        j.at("bearerBarWidth").get_to(m_bearerBarWidth);
    if (j.contains("autoCheckDigit"))
        j.at("autoCheckDigit").get_to(m_autoCheckDigit);
    if (j.contains("angle"))
        j.at("angle").get_to(m_angle);
    if (j.contains("segmentsByRow"))
        j.at("segmentsByRow").get_to(m_segmentsByRow);
    if (j.contains("sizeMode"))
        j.at("sizeMode").get_to(m_sizeMode);
    if (j.contains("sizeModules"))
        j.at("sizeModules").get_to(m_sizeModules);
    if (j.contains("modulesQZ"))
        j.at("modulesQZ").get_to(m_modulesQZ);
    if (j.contains("variable"))
        j.at("variable").get_to(m_variable);
}

bool lim::Barcode::prepareBarcode()
{
    // For bitmap output the origin is the top, left corner.
    BCSetDecoder(m_barCodeHandle, eDecoder_Software);
    BCSetMustFit(m_barCodeHandle, true);
    BCSetSizeMode(m_barCodeHandle, m_sizeMode);

    if (m_sizeMode == eSizeMode_CustomModuleWidth)
        BCSetModWidthN(m_barCodeHandle, m_sizeModules);

    BCSetBCType(m_barCodeHandle, m_typeBarcode);

    if (m_typeBarcode == eBC_RSSExpStacked) {
        BCSet_RSS_SegmPerRow(m_barCodeHandle, m_segmentsByRow);
        BCSet2DCompositeComponent(m_barCodeHandle, eCC_Auto);
    }

    if (m_autoCheckDigit || m_typeBarcode == eBC_ITF14) 
    {
        BCSetCDMethod(m_barCodeHandle, eCDStandard);
    } 
    else
        BCSetCDMethod(m_barCodeHandle, eCDNone);

    if (m_typeBarcode == eBC_GS1_128 ||
        (m_typeBarcode == eBC_Code128 && m_code.find('é')) ||
        m_typeBarcode == eBC_RSSExpStacked) 
    {
        BCSetTranslateEsc(m_barCodeHandle, TRUE);
        std::wstring key = std::wstring(L"é");
        std::wstring repl = std::wstring(L"\\F");
        m_code = std::regex_replace(m_code, std::wregex(key), repl);

        BCSetAutoCorrect(m_barCodeHandle, TRUE);
        BCSet_CBF_Format(m_barCodeHandle, eCBFPr_UCCEAN);
        m_code.erase(std::remove(m_code.begin(), m_code.end(), '('), m_code.end());
        m_code.erase(std::remove(m_code.begin(), m_code.end(), ')'), m_code.end());
    }
    else if (m_typeBarcode == eBC_DataMatrix)
    {
        if (m_DMFormat == eDMPr_UCCEAN) 
        {
            BCSetTranslateEsc(m_barCodeHandle, TRUE);
            std::wstring key = std::wstring(L"é");
            std::wstring repl = std::wstring(L"\\F");
            m_code = std::regex_replace(m_code, std::wregex(key), repl);

            BCSetAutoCorrect(m_barCodeHandle, TRUE);
            m_code.erase(std::remove(m_code.begin(), m_code.end(), '('), m_code.end());
            m_code.erase(std::remove(m_code.begin(), m_code.end(), ')'), m_code.end());
        }
    } 
    else if (m_typeBarcode == eBC_QRCode)
    {
        if (m_QRFormat == eQRPr_UCCEAN) 
        {
            BCSetTranslateEsc(m_barCodeHandle, TRUE);
            std::wstring key = std::wstring(L"é");
            std::wstring repl = std::wstring(L"\\F");
            m_code = std::regex_replace(m_code, std::wregex(key), repl);

            BCSetAutoCorrect(m_barCodeHandle, TRUE);
            m_code.erase(std::remove(m_code.begin(), m_code.end(), '('), m_code.end());
            m_code.erase(std::remove(m_code.begin(), m_code.end(), ')'), m_code.end());
        }
    } 
    else if (m_typeBarcode == eBC_ITF14) {
        if (m_code.length() > 13) {
            m_code = m_code.substr(0, 12);
        }
        m_autoCheckDigit = true;
        if (!m_rectangleGuard) {
            BCSetBearerBarWidth(m_barCodeHandle, m_bearerBarWidth);
            BCSetBearerBarType(m_barCodeHandle, eBearerBar_TopAndBottom);

            RECTD rectD;

            rectD.left = m_modulesQZ;
            rectD.top = 0;
            rectD.right = m_modulesQZ;
            rectD.bottom = 0;

            BCSetQuietZone(m_barCodeHandle, &rectD, eMUModules);
        }
        else 
        {
            BCSetBearerBarWidth(m_barCodeHandle, m_bearerBarWidth);
            BCSetBearerBarType(m_barCodeHandle, eBearerBar_Rectangle);

            RECTD rectD;

            rectD.left = m_modulesQZ;
            rectD.top = 0;
            rectD.right = m_modulesQZ;
            rectD.bottom = 0;

            BCSetQuietZone(m_barCodeHandle, &rectD, eMUModules);
        }
    }
    else 
    {
        BCSetBearerBarType(m_barCodeHandle, eBearerBar_None);
        BCSetBearerBarWidth(m_barCodeHandle, 0);

        if (m_typeBarcode == eBC_QRCode) {
            BCSet_QR_ECLevel(m_barCodeHandle, m_QRECLevel);
            BCSet_QR_Format(m_barCodeHandle, m_QRFormat);
        }
        else if (m_typeBarcode == eBC_DataMatrix) {
            BCSet_DM_Format(m_barCodeHandle, m_DMFormat);
            BCSet_DM_Rectangular(m_barCodeHandle, false);
        }
    }

    if (!m_showLabel)
        BCSetPrintText(m_barCodeHandle, false, false);
    else {
        BCSetFontHeight(m_barCodeHandle, m_fontSize);
        BCSetPrintText(m_barCodeHandle, true, false);
    }

    BCSetCodePage(m_barCodeHandle, eCodePage_UTF8);
    std::wstring code = std::wstring(m_code.begin(), m_code.end());

    m_errorCode = BCSetTextW(m_barCodeHandle, code.c_str(), code.length());

    BCSetDPI(m_barCodeHandle, 96, 96);
    BCSetFontHeight(m_barCodeHandle, m_fontSize);
    BCSetBarWidthReduction(m_barCodeHandle, m_bwr, eMUPercent);

    if (m_errorCode == 0) {
        std::wstring fontName = std::wstring(m_fontName.begin(), m_fontName.end());
        BCSetFontNameW(m_barCodeHandle, fontName.c_str());
        BCSetDrawMode(m_barCodeHandle, true);

        m_errorCode = BCCreate(m_barCodeHandle);
    }

    return m_errorCode == 0;
}

void lim::Barcode::draw(cv::Mat& img)
{
    if (prepareBarcode())
    {
        cv::Rect& rect = getRect();

        RECT r;
        r.left = 0;
        r.top = 0;
        r.right = rect.width;
        r.bottom = rect.height;

        HBITMAP hbmp = CreateBitmap(rect.width, rect.height, 1, 0, 0);
        HDC hdcimage = CreateCompatibleDC(NULL);
        SelectObject(hdcimage, hbmp);
    
        m_errorCode = BCDraw(m_barCodeHandle, hdcimage, &r);
        double realWidth, realHeight;
        BCGetBarcodeSize(m_barCodeHandle, &r, hdcimage, eMUPixel, &realWidth, &realHeight);
        rect.width = realWidth;
        rect.height = realHeight;

        cv::Mat imgBC = cv::Mat(rect.height, rect.width, CV_8UC4, cv::Scalar::all(255));
        BITMAPINFOHEADER bi = { sizeof(bi), rect.width, -rect.height, 1, 32, BI_RGB };
        GetDIBits(hdcimage, hbmp, 0, rect.height, imgBC.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        //cv::imshow("Prueba", imgBC);
        //cv::waitKey(0);

        ObjGraph::rotate(imgBC, img, rect, m_angle);

        DeleteDC(hdcimage);
        DeleteObject(hbmp);
    }
}

// ----------------------------------------------------------------------------------------
// Image
// ----------------------------------------------------------------------------------------

const json lim::Image::toJSON() const
{
    const cv::Rect& r = getRect();

    json j;

    if (m_embeded) {
        std::vector<uchar> buf;
        cv::imencode(".png", m_img, buf);
        std::string encoded_png;
        auto base64_png = reinterpret_cast<const unsigned char*>(buf.data());
        encoded_png = base64_encode(base64_png, buf.size());

        j = {
            { "type", "image" },
            { "x", r.x },
            { "y", r.y },
            { "width", r.width },
            { "height", r.height },
            { "angle", m_angle },
            { "embeded", m_embeded },
            { "path", "" },
            { "image", encoded_png },
        };
    }
    else {
        j = {
            { "type", "image" },
            { "x", r.x },
            { "y", r.y },
            { "width", r.width },
            { "height", r.height },
            { "angle", m_angle},
            { "embeded", m_embeded },
            { "path", m_path },
            { "image", "" },
        };
    }

    return j;
}

void lim::Image::from_json(const json& j)
{
    ObjGraph::from_json(j);
    if (j.contains("path"))
        j.at("path").get_to(m_path);
    if (j.contains("embeded"))
        j.at("embeded").get_to(m_embeded);
    if (j.contains("angle"))
        j.at("angle").get_to(m_angle);
    if (m_embeded) {
        std::string image;
        if (j.contains("image"))
            j.at("image").get_to(image);

        // Decodificar de base64
        std::string dec_jpg = base64_decode(image);
        std::vector<uchar> data(dec_jpg.begin(), dec_jpg.end());
        m_img = cv::imdecode(cv::Mat(data), 1);
    }
    else {
        m_img = cv::imread(m_path);
    }
    // Es menos costoso convertir sobre otra imagen que en la propia
    cv::Mat dst;
    cv::cvtColor(m_img, dst, cv::COLOR_BGR2BGRA);
    m_img = dst;
}

void  lim::Image::draw(cv::Mat& img)
{
    if(m_img.rows==0 && m_path!="")
        m_img = cv::imread(m_path);
    cv::Rect& rect = getRect();
    //cv::cvtColor(m_img, m_img, cv::COLOR_BGR2BGRA);

    if (m_img.rows != rect.height || m_img.cols != rect.height) {
        cv::Mat imgRes;
        cv::resize(m_img, imgRes, rect.size());
        ObjGraph::rotate(imgRes, img, rect, m_angle);
    }
    else
    {
        ObjGraph::rotate(m_img, img, rect, m_angle);
    }
}

// ----------------------------------------------------------------------------------------
// LimLayout
// ----------------------------------------------------------------------------------------

void lim::LimLayout::addVariable(std::unique_ptr<LimVar>&& variable)
{
    m_variables.insert( std::pair<std::string, std::unique_ptr<LimVar>>(variable->name(), std::move(variable)) );
}

void lim::LimLayout::readFromFile(const std::string& fileName)
{
    std::ifstream f(fileName);
    if (f.is_open()) {
        json j;
        f >> j;
        f.close();

        if (j.contains("width"))
            j.at("width").get_to(m_width);
        if (j.contains("height"))
            j.at("height").get_to(m_height);

        if (j.contains("variables")) {
            json j_list;
            j.at("variables").get_to(j_list);
            for (auto i = 0; i < j_list.size(); ++i) {
                json jItem = j_list[i];
                if (jItem.contains("type")) {
                    std::string vType;
                    jItem.at("type").get_to(vType);
                    if (vType == "text") {
                        std::unique_ptr<LimStringVar> obj = std::make_unique<LimStringVar>("");
                        obj->from_json(jItem);
                        addVariable(std::move(obj));
                    }
                }
            }
        }

        if (j.contains("objects")) {
            json j_list;
            j.at("objects").get_to(j_list);
            for (auto i = 0; i < j_list.size(); ++i) {
                json jItem = j_list[i];
                if (jItem.contains("type")) {
                    std::string vType;
                    jItem.at("type").get_to(vType);
                    std::unique_ptr<ObjGraph> obj{ nullptr };

                    if (vType == "line") {
                        obj = std::make_unique<Line>();
                        obj->from_json(jItem);
                    }
                    else if (vType == "rectangle") {
                        obj = std::make_unique<Rectangle>();
                        obj->from_json(jItem);
                    }
                    else if (vType == "ellipse") {
                        obj = std::make_unique<Ellipse>();
                        obj->from_json(jItem);
                    }
                    else if (vType == "text") {
                        obj = std::make_unique<Text>();
                        obj->from_json(jItem);
                    }
                    else if (vType == "barcode") {
                        obj = std::make_unique<Barcode>();
                        obj->from_json(jItem);
                    }
                    else if (vType == "image") {
                        obj = std::make_unique<Image>();
                        obj->from_json(jItem);
                    }

                    if(obj)
                        m_objects.push_back(std::move(obj));
                }
            }
        }
    }
}

std::wstring lim::LimLayout::getVariableValue(std::string_view variable)
{
    std::string varName{ variable };
    std::wstring result;
    std::map<std::string, std::unique_ptr<LimVar>>::const_iterator it = m_variables.find(varName);
    if (it != m_variables.end())
        result = it->second->getVariableValue();

/* cuando era vector
    auto it = std::find_if(m_variables.begin(), m_variables.end(), [&variable](std::unique_ptr<LimVar>& var) { return var.get()->name() == variable; });
    if (it != m_variables.end())
    {
        result = it->get()->getVariableValue();
    }
*/
    return result;
}



void lim::LimLayout::draw()
{
    if (!img) {
        img = new cv::Mat(m_width, m_height, CV_8UC4);
    }

    *img = cv::Scalar::all(255); // Pinta en blanco

    calcVariables();

    for (const auto& obj : m_objects) 
    {
        // Mira si tiene variable
        if (dynamic_cast<lim::Text*>(obj.get())) {
            auto txt = dynamic_cast<lim::Text*>(obj.get());
            std::string varName = txt->variable();
            if (varName.size() > 0) {
                txt->setText(getVariableValue(varName));;
            }
        }
        else if (dynamic_cast<lim::Barcode*>(obj.get())) {
            auto bc = dynamic_cast<lim::Barcode*>(obj.get());
            std::string varName = bc->variable();
            if (varName.size() > 0) {
                bc->setCode(getVariableValue(varName));;
            }
        }

        obj->draw(*img);
    }
}

void lim::LimLayout::calcVariables()
{
    for (auto itr = m_variables.begin(); itr != m_variables.end(); ++itr)
    {
        lim::LimVar* var = itr->second.get();
        if (var)
            var->setValue( var->calcValue() );
    }
}

// ----------------------------------------------------------------------------------------
