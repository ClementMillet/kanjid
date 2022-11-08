#pragma once 
#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <algorithm>

class KanjiInfo
{
    private:
        CURL *curl = curl_easy_init();
        tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
        std::string _ocr_kanji = "漢字";
        std::string _kanji = "漢字";
        std::string _spelling = "かんじ";
        std::string _meaning = "Character";
    
    public:
        KanjiInfo();
        ~KanjiInfo();
        std::string get_kanji();
        std::string get_spelling();
        std::string get_meaning();
        void img2string(Pix *img);
        Pix* open_img(char* img_path);
        void get_jisho();
};