#include <iostream>
#include <string>
#include <queue>
#include <set>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <chrono>


void bfs(CURL *curl, std::string &node, int limit);

size_t my_write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
    size_t totalBytes = size * nmemb;
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(buffer), totalBytes);
    return totalBytes;
}



int main(int argc, char* argv[]){
    
    if (argc < 3) {
        std::cerr << "Ensure first node name and levels to traverse is provided";
        return 1;
    }

    std::string firstNode = argv[1];
    int toCrawl = std::stoi(argv[2]);

    CURL *curl = curl_easy_init();

    bfs(curl, firstNode, toCrawl);

    curl_easy_cleanup(curl);
    return 0; 
}  


void bfs(CURL *curl, std::string &node, int limit){
    std::queue<std::string> q;
    std::set<std::string> nameSet; 

    auto start = std::chrono::high_resolution_clock::now();

    q.push(node);
    nameSet.insert(node);
    int level = 0;
    int processed = 0;

    while (!q.empty() && level != limit){
        // std::cout << "Level: " << level << "\n";

        int levelLength = q.size();
        
        // level by level
        for (int i = 0; i < levelLength; i++){
            
            std::string currNode = q.front(); // pop is different in c++ ??? 
            // std::cout << "Processing node: " << currNode << "\n"; Commented out std::statements for bench
            processed++;
            std::string url;
            std::string myjson; 

            // build url and call curl
            char* encoded = curl_easy_escape(curl, currNode.c_str(), currNode.length());
            if (encoded){
                std::string encodedName(encoded);
                url = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + encodedName;
                curl_free(encoded);
            } else {
                url = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + currNode;
            }
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&myjson);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_data);
            CURLcode result = curl_easy_perform(curl);
            if (result != CURLE_OK) {
                std::cout << "Curl call failed\n";
                q.pop();
                continue;
            }

            using namespace rapidjson;            
            Document doc;
            // random safety checks 
            doc.Parse(myjson.c_str());
            if (doc.HasParseError() || !doc.HasMember("neighbors") || !doc["neighbors"].IsArray()) {
                q.pop();
                continue;;
            }

            auto neighbors = doc["neighbors"].GetArray();

            for (const rapidjson::Value&n : neighbors) {
                std::string neighbor = n.GetString();

                if (nameSet.find(neighbor) == nameSet.end()){
                    q.push(neighbor);
                    nameSet.insert(neighbor);
                }
            }
            q.pop(); // remove the node we just processed
        }
        
        level++;

    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Exec time: " << elapsed.count() << std::endl;
    std::cout << "Visited Nodes Count: " << processed << "\n";
    std::cout << "Total levels traversed: " << limit << "\n";
}
