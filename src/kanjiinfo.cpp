#include"kanjiinfo.hpp"

void process_str(std::string &str)
{
    str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

std::size_t callback(
            const char* in,
            std::size_t size,
            std::size_t num,
            std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }

KanjiInfo::KanjiInfo()
{
    if (api->Init(NULL, "Japanese")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }
    api->SetPageSegMode(tesseract::PSM_SINGLE_WORD);
}

KanjiInfo::~KanjiInfo()
{
    delete(api);
}

std::string KanjiInfo::get_kanji()
{
    return _kanji;
}

std::string KanjiInfo::get_spelling()
{
    return _spelling;
}

std::string KanjiInfo::get_meaning()
{
    return _meaning;
}

Pix* KanjiInfo::open_img(char* img_path)
{
    Pix* img = pixRead(img_path);
    return img;
}

void KanjiInfo::img2string(Pix* img)
{
    api->SetImage(img);
    _ocr_kanji = api->GetUTF8Text();
    _ocr_kanji = _ocr_kanji.substr(0,_ocr_kanji.length()-1);
    std::cout << _ocr_kanji << std::endl;
}

void KanjiInfo::get_jisho()
{
    char * http_kanji = curl_easy_escape(curl, _ocr_kanji.c_str(), 0);
    std::string url("https://jisho.org/api/v1/search/words/?keyword=" + std::string(http_kanji));
    delete(http_kanji);
    long httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode == 200)
    {
        std::cout << "\nGot successful response from " << url << std::endl;

        Json::Value jsonData;
        Json::Reader jsonReader;

        if (jsonReader.parse(*httpData.get(), jsonData))
        {
            std::cout << "Successfully parsed JSON data" << std::endl;

            // Get information about the kanji from the json data
            _kanji = jsonData["data"][0]["japanese"][0]["word"].toStyledString();
            _spelling = jsonData["data"][0]["japanese"][0]["reading"].toStyledString();
            _meaning = jsonData["data"][0]["senses"][0]["english_definitions"].toStyledString();
            process_str(_kanji);
            process_str(_spelling);
            process_str(_meaning);
            std::cout << _kanji.c_str() << std::endl;
            std::cout << _spelling.c_str() << std::endl;
            std::cout << _meaning << std::endl;
        }
    }
}

