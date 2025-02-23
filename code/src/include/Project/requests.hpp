#include <curl/curl.h>
#include <stdexcept>
#include <string>

namespace requests {
    enum METHOD {
        POST,
        GET
    };

    struct Response {
        std::string data;
        CURLcode status;

        std::string decodeStatus(){
            switch ((int)status) {
                case 100: return "Continue";
                case 101: return "Switching Protocol";
                case 102: return "Processing";
                case 103: return "Early Hints";
                case 200: return "OK";
                case 201: return "Created";
                case 202: return "Accepted";
                case 203: return "Non-Authoritative Information";
                case 204: return "No Content";
                case 205: return "Reset Content";
                case 206: return "Partial Content";
                case 207: return "Multi-Status";
                case 208: return "Already Reported";
                case 226: return "IM Used";
                case 300: return "Multiple Choice";
                case 301: return "Moved Permanently";
                case 302: return "Found";
                case 303: return "See Other";
                case 304: return "Not Modified";
                case 305: return "Use Proxy";
                case 306: return "unused";
                case 307: return "Temporary Redirect";
                case 308: return "Permanent Redirect";
                case 400: return "Bad Request";
                case 401: return "Unauthorized";
                case 402: return "Payment Required";
                case 403: return "Forbidden";
                case 404: return "Not Found";
                case 405: return "Method Not Allowed";
                case 406: return "Not Acceptable";
                case 407: return "Proxy Authentication Required";
                case 408: return "Request Timeout";
                case 409: return "Conflict";
                case 410: return "Gone";
                case 411: return "Length Required";
                case 412: return "Precondition Failed";
                case 413: return "Payload Too Large";
                case 414: return "URI Too Long";
                case 415: return "Unsupported Media Type";
                case 416: return "Range Not Satisfiable";
                case 417: return "Expectation Failed";
                case 418: return "I'm a teapot";
                case 421: return "Misdirected Request";
                case 422: return "Unprocessable Entity";
                case 423: return "Locked";
                case 424: return "Failed Dependency";
                case 425: return "Too Early";
                case 426: return "Upgrade Required";
                case 428: return "Precondition Required";
                case 429: return "Too Many Requests";
                case 431: return "Request Header Fields Too Large";
                case 451: return "Unavailable For Legal Reasons";
                case 501: return "Not Implemented";
                case 502: return "Bad Gateway";
                case 503: return "Service Unavailable";
                case 504: return "Gateway Timeout";
                case 505: return "HTTP Version Not Supported";
                case 506: return "Variant Also Negotiates";
                case 507: return "Insufficient Storage";
                case 508: return "Loop Detected";
                case 510: return "Not Extended";
                case 511: return "Network Authentication Required";
                case 500: return "Internal Server Error";
            }
            return "Unknown";
        }
    };

    class Request {

            CURL *curl;

        public:
            std::string url;
            METHOD method;
            std::string payload;
        
            Response response(std::string content_type = "text/plain"){
                std::string responseString;
                CURLcode httpStatus;

                switch (method){
                    case POST:{
                        
                        struct curl_slist *slist1;
                        slist1 = NULL;
                        slist1 = curl_slist_append(slist1, ("Content-Type: " + content_type).c_str());

                        curl_mass_init(responseString);
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
                        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.38.0");
                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
                        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
                        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

                        httpStatus = curl_easy_perform (curl);

                        curl_slist_free_all(slist1);
                        slist1 = NULL;

                        return {responseString, httpStatus};
                    }
                    case GET:{
                        curl_mass_init(responseString);
                        
                        httpStatus = curl_easy_perform (curl);
                        return {responseString, httpStatus};
                    }
                    default:
                        break;
                }
            }

            Request(
                std::string url, 
                METHOD method, 
                std::string payload = ""
            ): url(url), method(method), payload(payload) {
                curl = curl_easy_init();

                if(!curl){
                    throw std::runtime_error("CURL is not initialized");
                }
            }

            Request(){
                curl = curl_easy_init();

                if(!curl){
                    throw std::runtime_error("CURL is not initialized");
                }
            }
            ~Request(){
                curl_easy_cleanup(curl);
            }

        private:

            static unsigned getFunction(void *ptr, unsigned size, unsigned nmemb, std::string* data) {
                data->append((char*) ptr, size * nmemb);
                return size * nmemb;
            }

            void curl_mass_init(std::string &responseString){
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
                curl_easy_setopt(curl, CURLOPT_URL, url.data());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getFunction);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
            }
    };
};