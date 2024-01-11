// DemoGraphObjects.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

//#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <chrono>
#include "LimLayout.h"
#include "ToolsJosep.h"
#include "LimVariable.h"

int main()
{
    setlocale(LC_ALL, "es_ES.utf-8");

    ERRCODE err = BCLicenseMeW(
        L"Mem: V.L. LIMITRONIC, S.L. ES-12006",
        eLicKindDeveloper,
        1,
        L"2354B6AC997E6C2644E75FC8034E96F2",
        eLicProd2D);
    
    auto start_time1 = std::chrono::high_resolution_clock::now();

    lim::LimLayout layout;
    layout.readFromFile("ejemplo1.json");

    //lim::LimLayout layout(512, 1024);
/*
    std::unique_ptr<lim::Line> l{ new lim::Line(cv::Rect(300, 100, 200, 0), 10) };
    layout.objects().push_back(std::move(l));

    std::unique_ptr<lim::Rectangle> r1{ new lim::Rectangle(cv::Rect(50, 100, 200, 150), 2) };
    layout.objects().push_back(std::move(r1));

    //std::unique_ptr<Text> t1{ new Text(cv::Rect(400, 200, 220, 80), -1, "Limitronic", "fonts/arial.ttf", 50, 0, true) };

    // Verificar que el archivo cpp es UTF-8 con BOM con el notepad
    std::unique_ptr<lim::Text> t1{ new lim::Text(cv::Rect(400, 200, 300, 150), L"Limitronic\n012345\nèáñôç", "Arial", 45, 0, true) };
    //t1->setText(l);
    layout.objects().push_back(std::move(t1));

    std::unique_ptr<lim::Ellipse> c1{ new lim::Ellipse(cv::Rect(550, 100, 200, 150), 2) };
    layout.objects().push_back(std::move(c1));

    std::unique_ptr<lim::Barcode> b1{ new lim::Barcode(cv::Rect(10, 300, 300, 150) ) };
    //b1->setFontName("OCR-B 10 BT");
    b1->setCode("http://www.limitronic.com");
    b1->setType(eBC_QRCode);
    b1->setFontName("Arial");
    b1->setFontSize(20);
    //b1->setAngle(90);
    layout.objects().push_back(std::move(b1));

    std::unique_ptr<lim::Image> i1{ new lim::Image(cv::Rect(600, 100, 200, 150), "images/img1.bmp", false)};
    layout.objects().push_back(std::move(i1));

    std::unique_ptr<lim::LimStringVar> v1 = std::make_unique<lim::LimStringVar>("var1");
    v1->setTextValue(L"Josep");
    layout.addVariable( std::move(v1) );
*/
    auto start_time = std::chrono::high_resolution_clock::now();

    layout.draw();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;
    std::cout <<  "\n\nTime to draw: " << time / std::chrono::milliseconds(1) << " ms to run.\n\n";

    time = end_time - start_time1;
    std::cout << "\n\nTotal time: " << time / std::chrono::milliseconds(1) << " ms to run.\n\n";

    //layout.writeToFile("ejemplo1.json");

    cv::Mat* m = layout.getImage();
    if (m != nullptr) {
        // Para guarda monocromo
        cv::cvtColor(*m, *m, cv::COLOR_BGR2GRAY);
        save_bmp1bpp(*m, "salida_mono.bmp");

        cv::imwrite("salida_24.bmp", *m);
        cv::imshow("Prueba Layout", *m);
        cv::waitKey(0);
    }

    return 0;
}
