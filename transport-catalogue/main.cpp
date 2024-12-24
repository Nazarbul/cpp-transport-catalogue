#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    transport_catalogue::input::ReadForStream(catalogue, cin);
    transport_catalogue::output::ReadForStreamandWriteToStream(catalogue, cin, cout);   
}