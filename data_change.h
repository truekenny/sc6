#ifndef SC6_DATA_CHANGE_H
#define SC6_DATA_CHANGE_H

#include "uri.h"
#include "sem.h"
#include "block.h"
#include "data_structure.h"

struct torrent *deletePeer(struct firstByteData *firstByte, struct query *query);
struct torrent *updatePeer(struct firstByteData *firstByte, struct query *query);

#endif //SC6_DATA_CHANGE_H
