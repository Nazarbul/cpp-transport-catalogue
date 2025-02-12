#include "transport_catalogue.h"
#include "jsonreader.h"
#include "domain.h"
#include "map_render.h"
#include "request_handler.h"
 
using namespace std;



int main(){
    std:: vector <domain::StatRequest> stat_request;
    transport_catalogue::TransportCatalogue catalogue;
    map_render::Render_settings render_settings;

    JsonReader json_reader;
    request_handler::RequestHandler request_handler;
    json_reader = JsonReader(std::cin);
    json_reader.Parse(catalogue, render_settings, stat_request);
    request_handler = request_handler::RequestHandler();
    request_handler.execute_queries(catalogue, stat_request, render_settings);
    json::Print(request_handler.GetDocument(), std::cout); 
}