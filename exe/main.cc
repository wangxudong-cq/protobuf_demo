#include "graph_pb.h"

#include <fstream>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

std::string fw_name = "g_firmware.bin";
std::string in_name = "inputHeader.bin";
std::string out_name = "outputHeader.bin";

static void saveFile(fs::path outPath, const char* name, char* data, uint32_t size) {
    if (size == 0)
    {
        std::cerr << "no valid data with: " << name << std::endl;
        return;
    }
    
    fs::path outFW = outPath / name;
    std::ofstream out(outFW.string(), std::ios::binary | std::ios::out);
    if (!out.is_open()) {
        std::cerr << "open file failed with: " << outFW.string() << std::endl;
        return;
    }
    out.write((char*)data, size);
    std::cout << "Deserialize firmware exists: " << outFW.string() << std::endl;
    out.close();
}

int main(int argc, char** argv) {
    std::string basePath;
    std::string foldName;
    if (argc == 1)
    {
        std::cout << "please input a path to pack and serialize all firmwares! " << std::endl;
        return -1;
    }
    else if (argc == 2)
    {
        basePath = argv[1];
        std::cout << "serialize all firmwares with path: " << basePath << std::endl;
    }
    else if (argc == 3)
    {
        basePath = argv[1];
        foldName = argv[2];
        std::cout << "serialize all firmwares with path: " << basePath << std::endl;
        std::cout << "deserialize one firmwares with name: " << foldName << std::endl;
    }
    
    graphProto_p graph = graphProtoIns(basePath.c_str());

    if (foldName.empty())
    {
        return 0;
    }

    uint32_t size = 0;
    void* data = nullptr;
    uint32_t in_size = 0;
    void* in_data = nullptr;
    uint32_t out_size = 0;
    void* out_data = nullptr;
    deserializeForGraphProto(graph, foldName.c_str(), &size, &data, &in_size, &in_data, &out_size, &out_data);
    printf("firmware %s data size: %#x, data address:%#lx\n", foldName.c_str(), size, (uint64_t)data);

    fs::path outPath = fs::path(basePath) / foldName;
    try {
        if (fs::create_directories(outPath)) {
            std::cout << "Directory created: " << outPath << std::endl;
        } else {
            std::cout << "Directory already exists: " << outPath << std::endl;
        }
    } catch (fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    saveFile(outPath, fw_name.c_str(), (char*)data, size);
    saveFile(outPath, in_name.c_str(), (char*)in_data, in_size);
    saveFile(outPath, out_name.c_str(), (char*)out_data, out_size);

    graphProtoDel(&graph);

    return 0;
}