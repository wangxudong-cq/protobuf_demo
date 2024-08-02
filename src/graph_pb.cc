#include <filesystem>//c++17
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "graph.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "log.h"
#include "miniz.h"
#include "graph_pb.h"

#ifdef __cplusplus
extern "C" {
#endif
    struct _graphProto
    {
        char* fpb_;
        char* path_;
        void* graphs_;
    } graphProto_t;
    
    using namespace std;
    string fw_name = "g_firmware.bin";
    string in_name = "inputHeader.bin";
    string out_name = "outputHeader.bin";
    string pb_name = "firmware.pb";
    namespace fs = std::filesystem;

    static std::vector<unsigned char> compressData(const unsigned char* originalData, size_t originalSize) {
        mz_ulong compressedSize = compressBound(originalSize);
        std::vector<unsigned char> compressed(compressedSize);

        int result = compress(compressed.data(), &compressedSize, originalData, originalSize);

        if (result == Z_OK) {
            compressed.resize(compressedSize);
        } else {
            log_error("Compression failed with error code: %d", result);
        }

        return compressed;
    }

    static std::vector<unsigned char> decompressData(const unsigned char* compressedData, size_t compressedSize, size_t originalSize) {
        std::vector<unsigned char> decompressed(originalSize);

        int result = uncompress(decompressed.data(), &originalSize, compressedData, compressedSize);

        if (result != Z_OK) {
            log_error("Decompression failed with error code: %d", result);
        } else {
            decompressed.resize(originalSize);
        }

        return decompressed;
    }

    static std::vector<unsigned char> compressBinFile(const char* path, const char* file, uint32_t *fileSize) {
        std::vector<unsigned char> compressedData;
        compressedData.clear();
        fs::path basePath(path);
        fs::path filePath = basePath / file;

        if (!fs::exists(filePath))
        {
            if (strcmp(file, fw_name.c_str())==0)
            {
                log_error("Not exist successfully with name:%s ,file: %s", file, path);
            }
            else
            {
                log_warn("Not exist successfully with name:%s ,file: %s", file, path);
            }   
            return compressedData;
        }

        *fileSize = fs::file_size(filePath);
        std::ifstream inFile(filePath, std::ios::in | std::ios::binary);
        if (!inFile.is_open()) {
            log_error("Not opened successfully with file: %s", path);
            return compressedData;
        }

        std::vector<unsigned char> originalData(*fileSize);
        if (!inFile.read(reinterpret_cast<char*>(originalData.data()), *fileSize))
        {
            log_error("Not read successfully with file: %s", path);
            return compressedData;
        }
        log_debug("\r\033[K\n   originalData size: %#x", originalData.size());

        compressedData  = compressData(originalData.data(), originalData.size());
        inFile.close(); 
        return compressedData;
    }

    static bool serializeForGraphProto(const char* path, const char* file, graph::GraphProto* graph)
    {
        static uint32_t zz = 0;

        graph->set_graph_name(file);

        uint32_t fwSize = 0;
        std::vector<unsigned char> compressedFw = compressBinFile(path, fw_name.c_str(), &fwSize);
        graph->set_graph_size(fwSize);
        graph->set_graph_data(reinterpret_cast<char*>(compressedFw.data()), compressedFw.size());
        zz += compressedFw.size();
        log_debug("\r\033[K\n compressedData size: %#x", compressedFw.size());
        log_debug("\r\033[K\n  current total size: %#x", zz);

        uint32_t inSize = 0;
        std::vector<unsigned char> compressedIn = compressBinFile(path, in_name.c_str(), &inSize);
        graph->set_graph_in_size(inSize);
        graph->set_graph_in_data(reinterpret_cast<char*>(compressedIn.data()), compressedIn.size());
        zz += compressedIn.size();
        log_debug("\r\033[K\n compressedData size: %#x", compressedIn.size());
        log_debug("\r\033[K\n  current total size: %#x", zz);

        uint32_t outSize = 0;
        std::vector<unsigned char> compressedOut = compressBinFile(path, out_name.c_str(), &outSize);
        graph->set_graph_out_size(outSize);
        graph->set_graph_out_data(reinterpret_cast<char*>(compressedOut.data()), compressedOut.size());
        zz += compressedOut.size();
        log_debug("\r\033[K\n compressedData size: %#x", compressedOut.size());
        log_debug("\r\033[K\n  current total size: %#x", zz);
        return true;
    }

    static void serializeForGraphProtoAll(const fs::path& dir, const fs::path& base_dir, graph::GraphsProto* graphs)
    {
        for (const auto& entry : fs::recursive_directory_iterator(dir))
        {
            if (entry.is_regular_file() && entry.path().filename() == fw_name) {
                fs::path absolute_path = fs::absolute(entry.path());
                fs::path relative_path = fs::relative(entry.path(), base_dir);
                log_debug("Absolute Path: %s, Relative Path: %s", absolute_path.c_str(), relative_path.parent_path().c_str());
                
                graph::GraphProto* graph = graphs->add_graph_node();
                if (!serializeForGraphProto(absolute_path.parent_path().c_str(), relative_path.parent_path().c_str(), graph))
                {
                    log_error("Not add graph successfully with file: %s, name: %s", absolute_path.c_str(),relative_path.parent_path().c_str());
                }
            }
        }

        return;
    }

    static bool serializeForGraphsProto(graphProto_p pGpb)
    {
        const char* path_ = pGpb->path_;
        fs::path file_ = fs::path(path_) / pb_name;
        const char* fpb_ = file_.c_str();
        graph::GraphsProto* graphs = (graph::GraphsProto*)(pGpb->graphs_);
        serializeForGraphProtoAll(fs::path(path_), fs::path(path_), graphs);

        pGpb->fpb_ = (char*)malloc(strlen(fpb_) + 1);
        memset(pGpb->fpb_, 0, strlen(fpb_) + 1);
        strcpy(pGpb->fpb_, fpb_);

        log_debug("start serialize to file: %s", fpb_);

        std::ofstream output(fpb_, std::ios::binary);
        if (!output) {
            log_error("Not opened successfully with file: %s", fpb_);
            return false;
        }

        google::protobuf::io::OstreamOutputStream output_stream(&output);
        google::protobuf::io::CodedOutputStream coded_output(&output_stream);

        coded_output.SetSerializationDeterministic(true);
        if (graphs->SerializeToCodedStream(&coded_output))
        {
            log_error("Failed to serialize graphs.");
            return false;
        }
        return true;
    }

    static bool deserializeForGraphsProto(graphProto_p pGpb)
    {
        const char* fpb_ = pGpb->fpb_;
        const char* path_ = pGpb->path_;
        graph::GraphsProto* graphs = (graph::GraphsProto*)(pGpb->graphs_);
        log_debug("start deserialize from file: %s", fpb_);

        std::ifstream input(fpb_, std::ios::binary);
        if (!input) {
            log_error("Not opened successfully with file: %s", fpb_);
            return false;
        }

        google::protobuf::io::IstreamInputStream input_stream(&input);
        google::protobuf::io::CodedInputStream coded_input(&input_stream);

        coded_input.SetTotalBytesLimit(0x7fffffff);
        if (!graphs->ParseFromCodedStream(&coded_input)) {
            log_error("Failed to deserialize graphs.");
            return false;
        } 
        return true;
    }

    static void deserializeForBin(const unsigned char* compressPtr, uint32_t compressSize, uint32_t* size, void** data) {
        uint32_t fileSize = *size;

        if (fileSize == 0)
        {
            *data = nullptr;
            return;
        }

        *data = malloc(fileSize);
        if (*data == nullptr) {
            log_error("Failed to allocate memory.");
            return;
        }

        std::vector<unsigned char> decompressedData = decompressData(compressPtr, compressSize, fileSize);
        if (decompressedData.empty()) {
            log_error("Failed to decompress data.");
            free(*data);
            *data = nullptr;
            return;
        }
        else
        {
            memcpy(*data, decompressedData.data(), fileSize);
            log_debug("compressedData size %#x", compressSize);
            log_debug("decompressedData size %#x", decompressedData.size());
        }
    }

    bool deserializeForGraphProto(graphProto_p pGpb, const char* file, uint32_t* size_, void** data_, uint32_t* in_size_, void** in_data_, uint32_t* out_size_, void** out_data_)
    {
        const char* fpb_ = pGpb->fpb_;
        const char* path_ = pGpb->path_;
        graph::GraphsProto* graphs = (graph::GraphsProto*)(pGpb->graphs_);

        for (uint32_t i = 0; i < graphs->graph_node_size(); ++i)
        {
            graph::GraphProto* graph_ = graphs->mutable_graph_node(i);
            if (strcmp(graph_->graph_name().c_str(), file) == 0)
            {
                *size_ = graph_->graph_size();
                uint32_t compressSize = graph_->graph_data().size();
                const unsigned char* compressPtr = reinterpret_cast<const unsigned char*>(graph_->graph_data().data());
                deserializeForBin(compressPtr, compressSize, size_, data_);

                //!input
                *in_size_ = graph_->graph_in_size();
                compressSize = graph_->graph_in_data().size();
                compressPtr = reinterpret_cast<const unsigned char*>(graph_->graph_in_data().data());
                deserializeForBin(compressPtr, compressSize, in_size_, in_data_);

                
                //!output
                *out_size_ = graph_->graph_out_size();
                compressSize = graph_->graph_out_data().size();
                compressPtr = reinterpret_cast<const unsigned char*>(graph_->graph_out_data().data());
                deserializeForBin(compressPtr, compressSize, out_size_, out_data_);

                return true;
            }
        }
        return false;
    }

    static bool serializeFileExists(graphProto_p pGpb)
    {
        const char* fpb_ = pGpb->fpb_;
        const char* path_ = pGpb->path_;
        graph::GraphsProto* graphs = (graph::GraphsProto*)(pGpb->graphs_);
        log_debug("start check exist with path: %s", path_);

        for (const auto& entry : fs::recursive_directory_iterator(fs::path(path_)))
        {
            if (entry.is_regular_file() && entry.path().filename() == pb_name) {
                fs::path file = fs::absolute(entry.path());
                const char* fpb_ = file.c_str();

                pGpb->fpb_ = (char*)malloc(strlen(fpb_) + 1);
                memset(pGpb->fpb_, 0, strlen(fpb_) + 1);
                strcpy(pGpb->fpb_, fpb_);
                log_debug("found firmware with file: %s", fpb_);
                return true;
            }
        }

        return false;
    }

    graphProto_p graphProtoIns(const char* path) {
        log_info("\r\033[K\n%s", __func__);
        graphProto_p pins = (graphProto_p)malloc(sizeof(graphProto_t));

        GOOGLE_PROTOBUF_VERIFY_VERSION;
        pins->path_ = (char*)malloc(strlen(path) + 1);
        memset(pins->path_, 0, strlen(path) + 1);
        strcpy(pins->path_, path);
        pins->graphs_ = new graph::GraphsProto;

        if (serializeFileExists(pins))
        {
            log_debug("start deserialize");
            deserializeForGraphsProto(pins);
        }
        else
        {
            log_debug("start serialize");
            serializeForGraphsProto(pins);
            log_debug("start deserialize");
            deserializeForGraphsProto(pins);
        }

        return pins;
    }
    
    void graphProtoDel(graphProto_p* ppGpb) {
        google::protobuf::ShutdownProtobufLibrary();
        if ((*ppGpb) == nullptr)
        {
            log_fatal("cannot deleted a graphProto_t ins!");
            return;
        }
            
        graph::GraphsProto* graphs = static_cast<graph::GraphsProto*>((*ppGpb)->graphs_);
        delete graphs;
        (*ppGpb)->graphs_ = nullptr;

        free((*ppGpb)->fpb_);
        (*ppGpb)->fpb_ = nullptr;
    
        free((*ppGpb)->path_);
        (*ppGpb)->path_ = nullptr;

        free(*ppGpb);
        *ppGpb = nullptr;
        log_info("\r\033[K\n%s", __func__);
    }

#ifdef __cplusplus
}
#endif



    