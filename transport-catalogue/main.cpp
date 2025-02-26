#include "transport_catalogue.h"
#include "jsonreader.h"
#include "domain.h"
#include "map_render.h"
#include "request_handler.h"

using namespace std;

int main()
{
    transport_catalogue::TransportCatalogue catalogue;
    map_render::RenderSettings render_settings;
    jsonreader::JsonReader json_reader(std::cin, catalogue, render_settings);
    try
    {
    json_reader.FillCatalogue();
    const auto& stat_request = json_reader.GetStatRequest();
    const auto& node_render_settings = json_reader.GetRenderRequest();
    const auto map_render = json_reader.ParseNodeRequest(node_render_settings);
    request_handler::RequestHandler RequestHandler(catalogue, map_render);
    json_reader.Execute_Queries(stat_request, RequestHandler);
    }
    catch(const std::exception& e){
        std::cerr << "Ошибка: "s << e.what() << std::endl;
    }
}