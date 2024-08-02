#ifndef GRAPH_PB_
#define GRAPH_PB_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _graphProto* graphProto_p;

// struct _graphProto
// {
//     char* fpb_;
//     char* path_;
//     void* graphs_;
// } graphProto_t;

graphProto_p graphProtoIns(const char* path);
void graphProtoDel(graphProto_p* ppGpb);
bool deserializeForGraphProto(graphProto_p pGpb, const char* file, uint32_t* size_, void** data_, uint32_t* in_size_, void** in_data_, uint32_t* out_size_, void** out_data_);


#ifdef __cplusplus
}
#endif
#endif