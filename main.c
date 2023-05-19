#include <stdio.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

size_t write_callback(void* contents, size_t size, size_t nmemb, char** buffer) {
    size_t real_size = size * nmemb;
    *buffer = realloc(*buffer, real_size + 1);
    if (*buffer == NULL) {
        printf("Failed to allocate memory\n");
        return 0;
    }
    memcpy(*buffer, contents, real_size);
    (*buffer)[real_size] = '\0';
    return real_size;
}

int main() {
    CURL* curl;
    CURLcode res;
    char* xml_data = NULL;

    // Инициализация libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    // Установка URL для загрузки XML-файла
    char* url = "http://aiweb.cs.washington.edu/research/projects/xmltk/xmldata/data/pir/psd7003_pv.xml";
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Установка функции обратного вызова для записи полученных данных
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &xml_data);

        // Выполнение HTTP-запроса
        res = curl_easy_perform(curl);

        // Проверка на ошибки
        if (res != CURLE_OK) {
            printf("Failed to download XML file: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 1;
        }

        // Завершение сеанса libcurl
        curl_easy_cleanup(curl);
    }

    // Инициализация libxml2
    xmlInitParser();

    // Парсинг XML-данных
    xmlDoc* doc = xmlReadMemory(xml_data, strlen(xml_data), "noname.xml", NULL, 0);
    if (doc == NULL) {
        printf("Failed to parse XML data\n");
        xmlCleanupParser();
        free(xml_data);
        return 1;
    }

    // Получение корневого элемента документа
    xmlNode* root = xmlDocGetRootElement(doc);

    // Получение списка уникальных refid для каждой сущности
    xmlNode* refinfo;
    xmlChar* refid;
    xmlNodeSet* nodes = xmlXPathEvalExpression((xmlChar*)".//ProteinDatabase/ProteinEntry/reference/refinfo", xmlXPathNewContext(doc));
    for (int i = 0; i < nodes->nodeNr; ++i) {
        refinfo = nodes->nodeTab[i];
        refid = xmlNodeGetContent(xmlFirstElementChild(refinfo));
        printf("%s,", refid);
        xmlFree(refid);
    }
    printf("\n");

    // Освобождение ресурсов
    xmlXPathFreeNodeSet(nodes);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    free(xml_data);

    return 0;
}
