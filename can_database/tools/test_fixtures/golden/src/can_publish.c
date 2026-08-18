/* GENERATED FILE — do not edit by hand.
 * Source: can_messages.yaml, generator: tools/gen_can_database.py
 */


#include "can_database/can_publish.h"
#include "can_database/can_codec.h"
#include "can_database/can_ids.h"
#include "can_database_internal.h"

void CanDb_PlainMsg_Publish(const CanMsg_PlainMsg_t *m) {
    CanFrame_t frame;
    frame.id  = CAN_ID_PLAIN_MSG;
    frame.dlc = 4;
    CanCodec_PlainMsg_Encode(m, frame.data);
    CanDb_Send(CAN_BUS_PLAIN_MSG, &frame);
}

void CanDb_ScaledMsg_Publish(const CanMsg_ScaledMsg_t *m) {
    CanFrame_t frame;
    frame.id  = CAN_ID_SCALED_MSG;
    frame.dlc = 2;
    CanCodec_ScaledMsg_Encode(m, frame.data);
    CanDb_Send(CAN_BUS_SCALED_MSG, &frame);
}

void CanDb_PackedMsg_Publish(const CanMsg_PackedMsg_t *m) {
    CanFrame_t frame;
    frame.id  = CAN_ID_PACKED_MSG;
    frame.dlc = 3;
    CanCodec_PackedMsg_Encode(m, frame.data);
    CanDb_Send(CAN_BUS_PACKED_MSG, &frame);
}

